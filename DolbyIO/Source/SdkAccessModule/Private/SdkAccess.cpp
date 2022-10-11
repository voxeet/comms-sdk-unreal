#include "SdkAccess.h"
#include "Common.h"
#include "DeviceManagement.h"
#include "Modules/ModuleManager.h"
#include "SdkEventsObserver.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, SdkAccessModule)

static std::string ToStdString(const FString& String)
{
	return TCHAR_TO_UTF8(*String);
}

namespace Dolby
{
	using namespace dolbyio::comms;

	FSdkAccess::FSdkAccess(ISdkEventsObserver& Observer)
	    : Status(Observer), Observer(Observer), ExceptionHandler(Status) 
	{
		ExceptionHandler.NotifyIfThrows(
		    [this]()
		    {
			    static std::once_flag Flag;
			    std::call_once(Flag,
			                   []
			                   {
#if PLATFORM_WINDOWS
				                   static auto AlignedNew = +[](std::size_t Count, std::size_t Al)
				                   { return operator new(Count, static_cast<std::align_val_t>(Al)); };
				                   static auto AlignedDelete = +[](void* Ptr, std::size_t Al)
				                   { operator delete(Ptr, static_cast<std::align_val_t>(Al)); };
				                   sdk::set_app_allocator({operator new, AlignedNew, operator delete, AlignedDelete});
#endif
				                   sdk::log_settings LogSettings;
				                   LogSettings.sdk_log_level = log_level::INFO;
				                   LogSettings.media_log_level = log_level::OFF;
				                   LogSettings.log_directory = "";
				                   sdk::set_log_settings(LogSettings);
			                   });
		    });
	}

	FSdkAccess::~FSdkAccess() {}

	void FSdkAccess::Connect(const FToken& Token, const FConferenceName& Conf, const FUserName& User)
	{
		if (Token.IsEmpty() || Conf.IsEmpty() || User.IsEmpty())
		{
			Status.SetMsg("Client access token, conference name and user name cannot be empty");
			return;
		}
		if (Status.IsConnecting())
		{
			return;
		}
		ExceptionHandler.NotifyIfThrows(
		    [this, &Token, &Conf, &User]
		    {
			    if (Status.IsConnected())
			    {
				    Disconnect();
			    }

			    Status.OnConnecting();
			    Sdk.Reset(sdk::create(ToStdString(Token),
			                          [this](auto&& cb)
			                          {
				                          RefreshTokenCb.Reset(cb.release());
				                          Observer.OnRefreshTokenRequested();
			                          })
			                  .release());
			    Devices = MakeUnique<FDeviceManagement>(Sdk->device_management(), Observer, ExceptionHandler);
			    Connect(Conf, User);
		    });
	}

	void FSdkAccess::Connect(const FConferenceName& Conf, const FUserName& User)
	{
		using namespace dolbyio::comms::services;

		if (Conf == "demo")
		{
			return ConnectToDemoConference(User);
		}

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
			        }

			        conference::conference_options Options{};
			        Options.alias = ToStdString(Conf);
			        Options.params.spatial_audio_style = spatial_audio_style::shared;

			        return Sdk->conference().create(Options);
		        })
		    .then(
		        [this](auto&& ConferenceInfo)
		        {
			        conference::join_options Options{};
			        Options.constraints.audio = true;
			        Options.connection.spatial_audio = true;
			        return Sdk->conference().join(ConferenceInfo, Options);
		        })
		    .then([this](auto&&) { Status.OnConnected(); })
		    .on_error(ExceptionHandler);
	}

	void FSdkAccess::ConnectToDemoConference(const FUserName& User)
	{
		DemoParticipantIDs.Empty();

		services::session::user_info UserInfo{};
		UserInfo.name = ToStdString(User);

		Sdk->session()
		    .open(MoveTemp(UserInfo))
		    .then(
		        [this](auto&& User)
		        {
			        if (User.participant_id)
			        {
				        LocalParticipantID = User.participant_id->c_str();
			        }
			        return Sdk->conference().demo();
		        })
		    .then(
		        [this](auto&&)
		        {
			        Status.OnConnected();
			        return Sdk->conference().add_event_handler(
			            [this](const participant_added& Event)
			            { DemoParticipantIDs.Add(Event.participant.user_id.c_str()); });
		        })
		    .on_error(ExceptionHandler);
	}

	void FSdkAccess::Disconnect()
	{
		if (Status.IsConnected())
		{
			ExceptionHandler.NotifyIfThrows(
			    [this]
			    {
				    Status.OnDisconnected();
				    Sdk->conference()
				        .leave()
				        .then([this] { return Sdk->session().close(); })
				        .on_error(ExceptionHandler);
			    });
		}
	}

	void FSdkAccess::MuteInput(const bool bIsMuted)
	{
		if (Status.IsConnected())
		{
			ExceptionHandler.NotifyIfThrows([this, bIsMuted]()
			                                { Sdk->conference().mute(bIsMuted).on_error(ExceptionHandler); });
		}
	}

	void FSdkAccess::MuteOutput(const bool bIsMuted)
	{
		if (Status.IsConnected())
		{
			ExceptionHandler.NotifyIfThrows([this, bIsMuted]()
			                                { Sdk->conference().mute_output(bIsMuted).on_error(ExceptionHandler); });
		}
	}

	void FSdkAccess::SetInputDevice(const int Index)
	{
		if (Status.IsConnected())
		{
			ExceptionHandler.NotifyIfThrows([this, Index]() { Devices->SetInputDevice(Index); });
		}
	}

	void FSdkAccess::SetOutputDevice(const int Index)
	{
		if (Status.IsConnected())
		{
			ExceptionHandler.NotifyIfThrows([this, Index]() { Devices->SetOutputDevice(Index); });
		}
	}

	void FSdkAccess::UpdateViewPoint(const FVector& Position, const FRotator& Rotation)
	{
		if (Status.IsConnected())
		{
			ExceptionHandler.NotifyIfThrows(
			    [this, &Position, &Rotation]
			    {
				    spatial_audio_batch_update Update;

				    if (DemoParticipantIDs.Num())
				    {
					    static float Angle = 0;
					    Angle += 0.1f;
					    for (const auto& Participant : DemoParticipantIDs)
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
				    Sdk->conference().update_spatial_audio_configuration(MoveTemp(Update)).on_error(ExceptionHandler);
			    });
		}
	}

	void FSdkAccess::RefreshToken(const FToken& token)
	{
		if (Status.IsConnected())
		{
			ExceptionHandler.NotifyIfThrows(
			    [this, &token]
			    {
				    if (RefreshTokenCb)
				    {
					    (*RefreshTokenCb)(ToStdString(token));
				    }
			    });
		}
	}

	sdk* FSdkAccess::GetRawSdk()
	{
		return Sdk.Get();
	}
}
