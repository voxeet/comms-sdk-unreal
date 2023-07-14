// Copyright 2023 Dolby Laboratories

#include "DolbyIO.h"

#include "Utils/DolbyIOBroadcastEvent.h"
#include "Utils/DolbyIOConversions.h"
#include "Utils/DolbyIOErrorHandler.h"
#include "Utils/DolbyIOLogging.h"

using namespace dolbyio::comms;
using namespace DolbyIO;

void UDolbyIOSubsystem::Connect(const FString& ConferenceName, const FString& UserName, const FString& ExternalID,
                                const FString& AvatarURL, EDolbyIOConnectionMode ConnMode,
                                EDolbyIOSpatialAudioStyle SpatialStyle, int MaxVideoStreams,
                                EDolbyIOVideoForwardingStrategy VideoForwardingStrategy)
{
	using namespace dolbyio::comms::services;

	if (!CanConnect())
	{
		return;
	}
	if (ConferenceName.IsEmpty())
	{
		DLB_UE_WARN("Cannot connect - conference name cannot be empty");
		return;
	}

	ConnectionMode = ConnMode;
	SpatialAudioStyle = SpatialStyle;
	DLB_UE_LOG("Connecting to conference %s with user name \"%s\" (%s, %s)", *ConferenceName, *UserName,
	           *UEnum::GetValueAsString(ConnectionMode), *UEnum::GetValueAsString(SpatialAudioStyle));

	services::session::user_info UserInfo{};
	UserInfo.name = ToStdString(UserName);
	UserInfo.externalId = ToStdString(ExternalID);
	UserInfo.avatarUrl = ToStdString(AvatarURL);
	EmptyRemoteParticipants();

	Sdk->session()
	    .open(MoveTemp(UserInfo))
	    .then(
	        [this, ConferenceName = ToStdString(ConferenceName)](services::session::user_info&& User)
	        {
		        LocalParticipantID = ToFString(User.participant_id.value_or(""));

		        conference::conference_options Options{};
		        Options.alias = ConferenceName;
		        Options.params.spatial_audio_style = ToSdkSpatialAudioStyle(SpatialAudioStyle);
		        return Sdk->conference().create(Options);
	        })
	    .then(
	        [this, MaxVideoStreams, VideoForwardingStrategy](conference_info&& ConferenceInfo)
	        {
		        ConferenceID = ToFString(ConferenceInfo.id);
		        if (ConnectionMode == EDolbyIOConnectionMode::Active)
		        {
			        conference::join_options Options{};
			        Options.constraints.audio = true;
			        Options.constraints.video = bIsVideoEnabled;
			        Options.connection.spatial_audio = IsSpatialAudio();
			        Options.connection.max_video_forwarding = MaxVideoStreams;
			        Options.connection.forwarding_strategy =
			            VideoForwardingStrategy == EDolbyIOVideoForwardingStrategy::LastSpeaker
			                ? video_forwarding_strategy::last_speaker
			                : video_forwarding_strategy::closest_user;
			        return Sdk->conference().join(ConferenceInfo, Options);
		        }
		        else
		        {
			        conference::listen_options Options{};
			        Options.connection.spatial_audio = IsSpatialAudio();
			        Options.type = ConnectionMode == EDolbyIOConnectionMode::ListenerRegular ? listen_mode::regular
			                                                                                 : listen_mode::rts_mixed;
			        return Sdk->conference().listen(ConferenceInfo, Options);
		        }
	        })
	    .then(
	        [this](conference_info&& ConferenceInfo)
	        {
		        DLB_UE_LOG("Connected to conference ID %s with user ID %s", *ConferenceID, *LocalParticipantID);
		        SetSpatialEnvironment();
		        ToggleInputMute();
		        ToggleOutputMute();
	        })
	    .on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::DemoConference()
{
	if (!CanConnect())
	{
		return;
	}

	DLB_UE_LOG("Connecting to demo conference");
	ConnectionMode = EDolbyIOConnectionMode::Active;
	SpatialAudioStyle = EDolbyIOSpatialAudioStyle::Shared;
	EmptyRemoteParticipants();

	Sdk->session()
	    .open({})
	    .then(
	        [this](services::session::user_info&& User)
	        {
		        LocalParticipantID = ToFString(User.participant_id.value_or(""));
		        return Sdk->conference().demo(ToSdkSpatialAudioStyle(SpatialAudioStyle));
	        })
	    .then(
	        [this](conference_info&& ConferenceInfo)
	        {
		        DLB_UE_LOG("Connected to conference ID %s", *ToFString(ConferenceInfo.id));
		        SetSpatialEnvironment();
		        ToggleInputMute();
		        ToggleOutputMute();
	        })
	    .on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::Disconnect()
{
	if (!IsConnected())
	{
		return;
	}

	DLB_UE_LOG("Disconnecting");
	Sdk->conference()
	    .leave()
	    .then([this]() { return Sdk->session().close(); })
	    .then([this] { BroadcastEvent(OnDisconnected); })
	    .on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::UpdateStatus(conference_status Status)
{
	ConferenceStatus = Status;
	DLB_UE_LOG("Conference status: %s", *ToString(ConferenceStatus));

	switch (ConferenceStatus)
	{
		case conference_status::joined:
			BroadcastEvent(OnConnected, LocalParticipantID, ConferenceID);
			break;
	}
}

bool UDolbyIOSubsystem::CanConnect() const
{
	if (!Sdk)
	{
		DLB_UE_WARN("Cannot connect - not initialized");
		return false;
	}
	if (IsConnected())
	{
		DLB_UE_WARN("Cannot connect - already connected, please disconnect first");
		return false;
	}
	return true;
}

bool UDolbyIOSubsystem::IsConnected() const
{
	return ConferenceStatus == conference_status::joined;
}

bool UDolbyIOSubsystem::IsConnectedAsActive() const
{
	return IsConnected() && ConnectionMode == EDolbyIOConnectionMode::Active;
}

void UDolbyIOSubsystem::EmptyRemoteParticipants()
{
	FScopeLock Lock{&RemoteParticipantsLock};
	RemoteParticipants.Empty();
}

TArray<FDolbyIOParticipantInfo> UDolbyIOSubsystem::GetParticipants()
{
	TArray<FDolbyIOParticipantInfo> Ret;
	if (IsConnected())
	{
		FScopeLock Lock{&RemoteParticipantsLock};
		RemoteParticipants.GenerateValueArray(Ret);
	}
	return Ret;
}

void UDolbyIOSubsystem::UpdateUserMetadata(const FString& UserName, const FString& AvatarURL)
{
	if (!IsConnected())
	{
		return;
	}

	DLB_UE_LOG("Updating user metadata: UserName=%s AvatarURL=%s", *UserName, *AvatarURL)
	services::session::user_info UserInfo{};
	UserInfo.name = ToStdString(UserName);
	UserInfo.avatarUrl = ToStdString(AvatarURL);
	Sdk->session().update(MoveTemp(UserInfo)).on_error(DLB_ERROR_HANDLER);
}
