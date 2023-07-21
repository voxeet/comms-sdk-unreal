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
                                EDolbyIOVideoForwardingStrategy VideoForwardingStrategy, EDolbyIOVideoCodec VideoCodec)
{
	using namespace dolbyio::comms::services;

	if (!CanConnect(OnConnectError))
	{
		return;
	}
	if (ConferenceName.IsEmpty())
	{
		DLB_WARNING(OnConnectError, "Cannot connect - conference name cannot be empty");
		return;
	}

	ConnectionMode = ConnMode;
	SpatialAudioStyle = SpatialStyle;
	DLB_UE_LOG("Connecting to conference %s with user name \"%s\" (%s, %s, %s/%d, %s)", *ConferenceName, *UserName,
	           *UEnum::GetValueAsString(ConnectionMode), *UEnum::GetValueAsString(SpatialAudioStyle),
	           *UEnum::GetValueAsString(VideoForwardingStrategy), MaxVideoStreams,
	           *UEnum::GetValueAsString(VideoCodec));

	services::session::user_info UserInfo{};
	UserInfo.name = ToStdString(UserName);
	UserInfo.externalId = ToStdString(ExternalID);
	UserInfo.avatarUrl = ToStdString(AvatarURL);
	EmptyRemoteParticipants();

	Sdk->session()
	    .open(MoveTemp(UserInfo))
	    .then(
	        [this, ConferenceName = ToStdString(ConferenceName),
	         VideoCodec = ToSdkVideoCodec(VideoCodec)](services::session::user_info&& User)
	        {
		        LocalParticipantID = ToFString(User.participant_id.value_or(""));

		        conference::conference_options Options{};
		        Options.alias = ConferenceName;
		        Options.params.spatial_audio_style = ToSdkSpatialAudioStyle(SpatialAudioStyle);
		        Options.params.video_codec = VideoCodec;
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
	    .on_error(DLB_ERROR_HANDLER(OnConnectError));
}

void UDolbyIOSubsystem::DemoConference()
{
	if (!CanConnect(OnDemoConferenceError))
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
	    .on_error(DLB_ERROR_HANDLER(OnDemoConferenceError));
}

void UDolbyIOSubsystem::Disconnect()
{
	if (!IsConnected())
	{
		return;
	}

	DLB_UE_LOG("Disconnecting");
	Sdk->conference().leave().on_error(DLB_ERROR_HANDLER(OnDisconnectError));
}

void UDolbyIOSubsystem::UpdateStatus(conference_status Status)
{
#if PLATFORM_ANDROID
	bIsRtpStarted = false;
#endif

	ConferenceStatus = Status;
	DLB_UE_LOG("Conference status: %s", *ToString(ConferenceStatus));

	switch (ConferenceStatus)
	{
		case conference_status::joined:
			BroadcastEvent(OnConnected, LocalParticipantID, ConferenceID);
			break;
		case conference_status::left:
		case conference_status::error:
			Sdk->session()
			    .close()
			    .then([this] { BroadcastEvent(OnDisconnected); })
			    .on_error(DLB_ERROR_HANDLER(OnDisconnectError));
			break;
	}
}

bool UDolbyIOSubsystem::CanConnect(const FDolbyIOOnErrorDelegate& OnError) const
{
	if (!Sdk)
	{
		DLB_WARNING(OnError, "Cannot connect - not initialized");
		return false;
	}
	if (IsConnected())
	{
		DLB_WARNING(OnError, "Cannot connect - already connected, please disconnect first");
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
	Sdk->session().update(MoveTemp(UserInfo)).on_error(DLB_ERROR_HANDLER(OnUpdateUserMetadataError));
}

void UDolbyIOSubsystem::SendMessage(const FString& Message, const TArray<FString>& ParticipantIDs)
{
	if (!IsConnected())
	{
		DLB_WARNING(OnSendMessageError, "Cannot send message - not connected");
		return;
	}

	DLB_UE_LOG("Sending message %s", *Message);
	std::vector<std::string> SdkParticipantIDs;
	SdkParticipantIDs.reserve(ParticipantIDs.Num());
	for (const FString& ID : ParticipantIDs)
	{
		SdkParticipantIDs.emplace_back(ToStdString(ID));
	}
	Sdk->conference()
	    .send(ToStdString(Message), MoveTemp(SdkParticipantIDs))
	    .on_error(DLB_ERROR_HANDLER(OnSendMessageError));
}

void UDolbyIOSubsystem::Handle(const remote_participant_added& Event)
{
	if (!Event.participant.status)
	{
		return;
	}

	const FDolbyIOParticipantInfo Info = ToFDolbyIOParticipantInfo(Event.participant);
	DLB_UE_LOG("Participant status added: UserID=%s Name=%s ExternalID=%s Status=%s", *Info.UserID, *Info.Name,
	           *Info.ExternalID, *ToString(*Event.participant.status));
	{
		FScopeLock Lock{&RemoteParticipantsLock};
		RemoteParticipants.Emplace(Info.UserID, Info);
	}

	BroadcastEvent(OnParticipantAdded, Info.Status, Info);
	BroadcastRemoteParticipantConnectedIfNecessary(Info);
	ProcessBufferedVideoTracks(Info.UserID);
}

void UDolbyIOSubsystem::Handle(const remote_participant_updated& Event)
{
	if (!Event.participant.status)
	{
		return;
	}

	const FDolbyIOParticipantInfo Info = ToFDolbyIOParticipantInfo(Event.participant);
	DLB_UE_LOG("Participant status updated: UserID=%s Name=%s ExternalID=%s Status=%s", *Info.UserID, *Info.Name,
	           *Info.ExternalID, *ToString(*Event.participant.status));
	{
		FScopeLock Lock{&RemoteParticipantsLock};
		RemoteParticipants.FindOrAdd(Info.UserID) = Info;
	}

	BroadcastEvent(OnParticipantUpdated, Info.Status, Info);
	BroadcastRemoteParticipantConnectedIfNecessary(Info);
	BroadcastRemoteParticipantDisconnectedIfNecessary(Info);
}

void UDolbyIOSubsystem::BroadcastRemoteParticipantConnectedIfNecessary(const FDolbyIOParticipantInfo& ParticipantInfo)
{
	if (ParticipantInfo.Status == EDolbyIOParticipantStatus::OnAir)
	{
		BroadcastEvent(OnRemoteParticipantConnected, ParticipantInfo);
	}
}

void UDolbyIOSubsystem::BroadcastRemoteParticipantDisconnectedIfNecessary(
    const FDolbyIOParticipantInfo& ParticipantInfo)
{
	if (ParticipantInfo.Status == EDolbyIOParticipantStatus::Left ||
	    ParticipantInfo.Status == EDolbyIOParticipantStatus::Kicked)
	{
		BroadcastEvent(OnRemoteParticipantDisconnected, ParticipantInfo);
	}
}

void UDolbyIOSubsystem::Handle(const local_participant_updated& Event)
{
	if (!Event.participant.status)
	{
		return;
	}

	const FDolbyIOParticipantInfo Info = ToFDolbyIOParticipantInfo(Event.participant);
	DLB_UE_LOG("Local participant status updated: UserID=%s Name=%s ExternalID=%s Status=%s", *Info.UserID, *Info.Name,
	           *Info.ExternalID, *ToString(*Event.participant.status));

	BroadcastEvent(OnLocalParticipantUpdated, Info.Status, Info);
}

void UDolbyIOSubsystem::Handle(const conference_message_received& Event)
{
	const FString Message = ToFString(Event.message);
	FScopeLock Lock{&RemoteParticipantsLock};
	if (const FDolbyIOParticipantInfo* Sender = RemoteParticipants.Find(ToFString(Event.user_id)))
	{
		DLB_UE_LOG("Message received: \"%s\" from %s (%s)", *Message, *Sender->Name, *Sender->UserID);
		BroadcastEvent(OnMessageReceived, Message, *Sender);
	}
	else
	{
		DLB_UE_LOG("Message received: %s from unknown participant", *Message);
		BroadcastEvent(OnMessageReceived, Message, FDolbyIOParticipantInfo{});
	}
}
