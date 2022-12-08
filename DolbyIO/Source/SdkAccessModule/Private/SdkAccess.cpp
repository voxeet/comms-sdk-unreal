// Copyright 2022 Dolby Laboratories

#include "SdkAccess.h"

#include "Common.h"
#include "DeviceManagement.h"
#include "ErrorHandler.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Math/Rotator.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "SdkEventsObserver.h"

#include <dolbyio/comms/async_result.h>

IMPLEMENT_MODULE(Dolby::FSdkAccess, SdkAccessModule)

static std::string ToStdString(const FString& String)
{
	return TCHAR_TO_UTF8(*String);
}

namespace Dolby
{
	using namespace dolbyio::comms;

	FSdkAccess::FSdkAccess()
	{
		Status.SetStatus(EConferenceStatus::destroyed);
	}

	FSdkAccess::~FSdkAccess() {}

	void FSdkAccess::StartupModule()
	try
	{
#if PLATFORM_WINDOWS
		LoadDll("avutil-56.dll");
		LoadDll("dvclient.dll");
		LoadDll("dolbyio_comms_media.dll");
		LoadDll("dolbyio_comms_sdk.dll");

		static auto AlignedNew =
		    +[](std::size_t Count, std::size_t Al) { return operator new(Count, static_cast<std::align_val_t>(Al)); };
		static auto AlignedDelete =
		    +[](void* Ptr, std::size_t Al) { operator delete(Ptr, static_cast<std::align_val_t>(Al)); };
		sdk::set_app_allocator({operator new, AlignedNew, operator delete, AlignedDelete});
#endif
		sdk::log_settings LogSettings;
		LogSettings.sdk_log_level = log_level::INFO;
		LogSettings.media_log_level = log_level::OFF;
		LogSettings.log_directory = "";
		sdk::set_log_settings(LogSettings);
	}
	catch (...)
	{
		MakeHandler(__LINE__).RethrowAndUpdateStatus();
	}

	void FSdkAccess::LoadDll(const FString& Dll)
	{
		const static auto BaseDir =
		    FPaths::Combine(*IPluginManager::Get().FindPlugin("DolbyIO")->GetBaseDir(), TEXT("Binaries/Win64"));
		if (const auto Handle = FPlatformProcess::GetDllHandle(*FPaths::Combine(*BaseDir, *Dll)))
		{
			DllHandles.Add(Handle);
		}
		else
		{
			throw std::runtime_error{std::string{"Failed to load "} + ToStdString(Dll)};
		}
	}

	void FSdkAccess::ShutdownModule()
	{
		DLB_UE_LOG("Shutting down SdkAccessModule");
		for (auto Handle : DllHandles)
		{
			FPlatformProcess::FreeDllHandle(Handle);
		}
	}

	void FSdkAccess::ShutDown()
	{
		DLB_UE_LOG("Shutting down SdkAccess");
		WaitForDisconnect();
	}

	void FSdkAccess::WaitForDisconnect()
	{
		if (Sdk && Status.IsConnected())
			try
			{
				dolbyio::comms::wait(Sdk->conference().leave().then([this]() { return Sdk->session().close(); }));
			}
			catch (...)
			{
				MakeHandler(__LINE__).RethrowAndUpdateStatus();
			}
	}

	void FSdkAccess::SetObserver(ISdkEventsObserver* AnObserver)
	{
		Observer = AnObserver;
		Status.SetObserver(Observer);
	}

	void FSdkAccess::RefreshToken(const FToken& Token)
	try
	{
		if (RefreshTokenCb)
		{
			(*RefreshTokenCb)(ToStdString(Token));
			RefreshTokenCb.Reset(); // RefreshToken callback can be called only once
		}
		else
		{
			Initialize(Token);
		}
	}
	catch (...)
	{
		MakeHandler(__LINE__).RethrowAndUpdateStatus();
	}

	void FSdkAccess::Initialize(const FToken& Token)
	try
	{
		if (Status.IsConnected())
		{
			DLB_UE_LOG("SDK already initialized and conference connected - skipping initialization");
			return;
		}
		RefreshTokenCb.Reset();
		LocalParticipantID.Reset();
		ParticipantIDs.Empty();
		Sdk.Reset(sdk::create(ToStdString(Token),
		                      [this](auto&& cb)
		                      {
			                      DLB_UE_LOG("Refresh token requested");
			                      RefreshTokenCb.Reset(cb.release());
			                      Observer->OnRefreshTokenRequested();
		                      })
		              .release());
		Devices.Reset(MakeUnique<FDeviceManagement>(Sdk->device_management(), *Observer,
		                                            [this](int Id) { return MakeHandler(Id); })
		                  .Release());

		Sdk->conference()
		    .add_event_handler([this](const conference_status_updated& Event) { Status.SetStatus(Event.status); })
		    .on_error(MakeHandler(__LINE__));

		Sdk->conference()
		    .add_event_handler(
		        [this](const participant_added& Event)
		        {
			        ParticipantIDs.Add(Event.participant.user_id.c_str());
			        Observer->OnListOfRemoteParticipantsChanged(ParticipantIDs);
		        })
		    .on_error(MakeHandler(__LINE__));

		Sdk->conference()
		    .add_event_handler(
		        [this](const participant_updated& Event)
		        {
			        const auto& ParticipantStatus = Event.participant.status;
			        if (ParticipantStatus)
			        {
				        if (*ParticipantStatus == participant_status::left)
				        {
					        ParticipantIDs.Remove(Event.participant.user_id.c_str());
					        Observer->OnListOfRemoteParticipantsChanged(ParticipantIDs);
				        }
			        }
		        })
		    .on_error(MakeHandler(__LINE__));

		Sdk->conference()
		    .add_event_handler(
		        [this](const active_speaker_change& Event)
		        {
			        FParticipants ActiveSpeakers;
			        for (const auto& Speaker : Event.active_speakers)
			        {
				        ActiveSpeakers.Add(Speaker.c_str());
			        }
			        Observer->OnListOfActiveSpeakersChanged(ActiveSpeakers);
		        })
		    .on_error(MakeHandler(__LINE__));
	}
	catch (...)
	{
		MakeHandler(__LINE__).RethrowAndUpdateStatus();
	}

	FErrorHandler FSdkAccess::MakeHandler(int Id)
	{
		return FErrorHandler([this, Id](const FMessage& Msg)
		                     { Status.SetMsg(Msg + " {" + std::to_string(Id).c_str() + "}"); },
		                     [this]()
		                     {
			                     if (Status.IsConnected())
			                     {
				                     Status.SetStatus(EConferenceStatus::leaving);
				                     Disconnect();
			                     }
		                     });
	}

	void FSdkAccess::Connect(const FToken& Token, const FConferenceName& Conf, const FUserName& User)
	try
	{
		if (!Sdk)
		{
			Initialize(Token);
			if (!Sdk)
			{
				return Status.SetMsg("Enter valid token");
			}
		}
		if (Conf.IsEmpty() || User.IsEmpty())
		{
			return Status.SetMsg("Conference name and user name cannot be empty");
		}
		if (Status.IsConnected())
		{
			return Status.SetMsg("Must disconnect first");
		}

		bIsDemo = Conf == "demo";

		using namespace dolbyio::comms::services;
		services::session::user_info UserInfo{};
		UserInfo.name = ToStdString(User);

		Sdk->session()
		    .open(MoveTemp(UserInfo))
		    .then(
		        [this, Conf](auto&& User)
		        {
			        if (User.participant_id)
			        {
				        LocalParticipantID = User.participant_id->c_str();
				        Observer->OnLocalParticipantChanged(LocalParticipantID);
			        }

			        if (bIsDemo)
			        {
				        return Sdk->conference().demo();
			        }

			        conference::conference_options Options{};
			        Options.alias = ToStdString(Conf);
			        Options.params.spatial_audio_style = spatial_audio_style::shared;

			        return Sdk->conference().create(Options);
		        })
		    .then(
		        [this](auto&& ConferenceInfo)
		        {
			        if (bIsDemo)
			        {
				        return async_result<conference_info>{MoveTemp(ConferenceInfo)};
			        }

			        conference::join_options Options{};
			        Options.constraints.audio = true;
			        Options.connection.spatial_audio = true;
			        return Sdk->conference().join(ConferenceInfo, Options);
		        })
		    .on_error(MakeHandler(__LINE__));
	}
	catch (...)
	{
		MakeHandler(__LINE__).RethrowAndUpdateStatus();
	}

	void FSdkAccess::Disconnect()
	{
		ParticipantIDs.Empty();
		if (Sdk)
		{
			Sdk->conference().leave().then([this]() { return Sdk->session().close(); }).on_error(MakeHandler(__LINE__));
		}
	}

	void FSdkAccess::MuteInput(const bool bIsMuted)
	{
		if (Status.IsConnected())
		{
			Sdk->conference().mute(bIsMuted).on_error(MakeHandler(__LINE__));
		}
	}

	void FSdkAccess::MuteOutput(const bool bIsMuted)
	{
		if (Status.IsConnected())
		{
			Sdk->conference().mute_output(bIsMuted).on_error(MakeHandler(__LINE__));
		}
	}

	void FSdkAccess::SetInputDevice(const int Index)
	{
		Devices->SetInputDevice(Index);
	}

	void FSdkAccess::SetOutputDevice(const int Index)
	{
		Devices->SetOutputDevice(Index);
	}

	void FSdkAccess::UpdateViewPoint(const FVector& Position, const FRotator& Rotation)
	{
		if (!Status.IsConnected())
		{
			return;
		}

		spatial_audio_batch_update Update;

		if (bIsDemo)
		{
			static float Angle = 0;
			Angle += 0.01f;
			for (const auto& Participant : ParticipantIDs)
			{
				Update.set_spatial_position(ToStdString(Participant),
				                            Participant[0] == '1'
				                                ? spatial_position{FMath::Cos(Angle), 0, FMath::Sin(Angle)}
				                            : Participant[0] == '2' ? spatial_position{-1, 0, 0}
				                                                    : spatial_position{1, 0, 0});
			}
		}

		// The default SDK spatial settings expect meters as unit of length,
		// Unreal uses centimeters for scale, so we divide every length by 100.
		// The default SDK coordinate system expects position arguments to mean (in order) right, up and
		// forward. In Unreal, right axis is +Y, up is +Z and forward is +X.
		const auto ScaledPosition = Position / 100;
		Update.set_spatial_position(ToStdString(LocalParticipantID),
		                            {ScaledPosition.Y, ScaledPosition.Z, ScaledPosition.X});
		Update.set_spatial_direction({Rotation.Pitch, Rotation.Yaw, Rotation.Roll});
		Sdk->conference().update_spatial_audio_configuration(MoveTemp(Update)).on_error(MakeHandler(__LINE__));
	}

	void FSdkAccess::GetAudioLevels()
	{
		if (!Status.IsConnected())
		{
			return;
		}
		Sdk->conference()
		    .get_all_audio_levels()
		    .then(
		        [this](auto&& Levels)
		        {
			        FAudioLevels AudioLevels;
			        for (const audio_level& Level : Levels)
			        {
				        AudioLevels.Emplace(Level.participant_id.c_str(), Level.level);
			        }
			        Observer->OnAudioLevelsChanged(AudioLevels);
		        })
		    .on_error(MakeHandler(__LINE__));
	}

	sdk* FSdkAccess::GetRawSdk()
	{
		return Sdk.Get();
	}
}
