// Copyright 2023 Dolby Laboratories

#include "DolbyIOConversions.h"

#include "Misc/Base64.h"

namespace DolbyIO
{
	using namespace dolbyio::comms;

	std::string ToStdString(const FString& String)
	{
		return TCHAR_TO_UTF8(*String);
	}

	FString ToFString(const std::string& String)
	{
		return String.c_str();
	}

	FText ToFText(const std::string& String)
	{
		return FText::FromString(UTF8_TO_TCHAR(String.c_str()));
	}

	FString ToString(conference_status Status)
	{
		switch (Status)
		{
			case conference_status::creating:
				return "creating";
			case conference_status::created:
				return "created";
			case conference_status::joining:
				return "joining";
			case conference_status::joined:
				return "joined";
			case conference_status::leaving:
				return "leaving";
			case conference_status::left:
				return "left";
			case conference_status::destroyed:
				return "destroyed";
			case conference_status::error:
				return "error";
			default:
				return "unknown";
		};
	}

	FString ToString(participant_status Status)
	{
		switch (Status)
		{
			case participant_status::reserved:
				return "reserved";
			case participant_status::connecting:
				return "connecting";
			case participant_status::on_air:
				return "on_air";
			case participant_status::decline:
				return "decline";
			case participant_status::inactive:
				return "inactive";
			case participant_status::left:
				return "left";
			case participant_status::warning:
				return "warning";
			case participant_status::error:
				return "error";
			default:
				return "unknown";
		};
	}

	FString ToString(enum audio_device::direction Direction)
	{
		switch (Direction)
		{
			case audio_device::input:
				return "input";
			case audio_device::output:
				return "output";
			case audio_device::input_and_output:
				return "input&output";
			case audio_device::none:
			default:
				return "none";
		}
	}

	FString ToString(const audio_device& Device)
	{
		return FString::Printf(TEXT("%s, direction: %s, native_id: %s"), *ToFText(Device.name()).ToString(),
		                       *ToString(Device.direction()), *ToUnrealDeviceID(Device.native_id()));
	}

	EDolbyIOParticipantStatus ToEDolbyIOParticipantStatus(std::optional<participant_status> Status)
	{
		if (Status)
		{
			switch (*Status)
			{
				case participant_status::reserved:
					return EDolbyIOParticipantStatus::Reserved;
				case participant_status::connecting:
					return EDolbyIOParticipantStatus::Connecting;
				case participant_status::on_air:
					return EDolbyIOParticipantStatus::OnAir;
				case participant_status::decline:
					return EDolbyIOParticipantStatus::Decline;
				case participant_status::inactive:
					return EDolbyIOParticipantStatus::Inactive;
				case participant_status::left:
					return EDolbyIOParticipantStatus::Left;
				case participant_status::warning:
					return EDolbyIOParticipantStatus::Warning;
				case participant_status::error:
					return EDolbyIOParticipantStatus::Error;
			}
		}

		return EDolbyIOParticipantStatus::Unknown;
	}

	FDolbyIOParticipantInfo ToFDolbyIOParticipantInfo(const participant_info& Info)
	{
		FDolbyIOParticipantInfo Ret{};
		Ret.UserID = ToFString(Info.user_id);
		Ret.Name = ToFString(Info.info.name.value_or(""));
		Ret.ExternalID = ToFString(Info.info.external_id.value_or(""));
		Ret.AvatarURL = ToFString(Info.info.avatar_url.value_or(""));
		Ret.bIsListener = Info.type && *Info.type == participant_type::listener;
		Ret.bIsSendingAudio = Info.is_sending_audio.value_or(false);
		Ret.bIsAudibleLocally = Info.audible_locally.value_or(false);
		Ret.Status = ToEDolbyIOParticipantStatus(Info.status);

		FString DecodedExternalID;
		FBase64::Decode(Ret.ExternalID, DecodedExternalID);
		if (DecodedExternalID.StartsWith("{\"init-pos\": {\"x\": "))
		{
			Ret.bIsInjectedBot = true;

			FString Left;
			FString Right;

			DecodedExternalID.Split("\"x\": ", &Left, &Right);
			Right.Split(",", &Left, &Right);
			Ret.Location.Y = FCString::Atof(*Left) * ScaleCenti;

			Right.Split("\"y\": ", &Left, &Right);
			Right.Split(",", &Left, &Right);
			Ret.Location.Z = FCString::Atof(*Left) * ScaleCenti;

			Right.Split("\"z\": ", &Left, &Right);
			Right.Split(",", &Left, &Right);
			Ret.Location.X = -FCString::Atof(*Left) * ScaleCenti;

			Right.Split("\"r\": ", &Left, &Right);
			Right.Split(",", &Left, &Right);
			Ret.Rotation.Yaw = FCString::Atof(*Left);

			Ret.Rotation.Roll = 0;
			Ret.Rotation.Pitch = 0;
		}
		return Ret;
	}

	FDolbyIOVideoTrack ToFDolbyIOVideoTrack(const dolbyio::comms::video_track& Track)
	{
		FDolbyIOVideoTrack Ret;
		Ret.TrackID = ToFString(Track.sdp_track_id);
		Ret.ParticipantID = ToFString(Track.peer_id);
		Ret.bIsScreenshare = Track.is_screenshare;
		return Ret;
	}

	FDolbyIOVideoTrack ToFDolbyIOVideoTrack(
	    const dolbyio::comms::utils::participant_track_map::value_type& TrackMapItem)
	{
		FDolbyIOVideoTrack Ret;
		Ret.TrackID = ToFString(std::get<1>(TrackMapItem.second));
		Ret.ParticipantID = ToFString(TrackMapItem.first);
		Ret.bIsScreenshare = false;
		return Ret;
	}

	spatial_audio_style ToSdkSpatialAudioStyle(EDolbyIOSpatialAudioStyle SpatialAudioStyle)
	{
		switch (SpatialAudioStyle)
		{
			case EDolbyIOSpatialAudioStyle::Individual:
				return spatial_audio_style::individual;
			case EDolbyIOSpatialAudioStyle::Shared:
				return spatial_audio_style::shared;
			default:
				return spatial_audio_style::disabled;
		}
	}

	screen_share_content_info ToSdkContentInfo(EDolbyIOScreenshareEncoderHint EncoderHint,
	                                           EDolbyIOScreenshareMaxResolution MaxResolution,
	                                           EDolbyIOScreenshareDownscaleQuality DownscaleQuality)
	{
		screen_share_content_info Ret;

		switch (EncoderHint)
		{
			case EDolbyIOScreenshareEncoderHint::Detailed:
				Ret.hint = screen_share_content_info::encoder_hint::detailed;
				break;
			case EDolbyIOScreenshareEncoderHint::Text:
				Ret.hint = screen_share_content_info::encoder_hint::text;
				break;
			case EDolbyIOScreenshareEncoderHint::Fluid:
				Ret.hint = screen_share_content_info::encoder_hint::fluid;
				break;
			default:
				Ret.hint = screen_share_content_info::encoder_hint::unspecified;
		}

		switch (MaxResolution)
		{
			case EDolbyIOScreenshareMaxResolution::DownscaleTo1080p:
				Ret.resolution = screen_share_content_info::max_resolution::downscale_to_1080p;
				break;
			case EDolbyIOScreenshareMaxResolution::DownscaleTo1440p:
				Ret.resolution = screen_share_content_info::max_resolution::downscale_to_1440p;
				break;
			default:
				Ret.resolution = screen_share_content_info::max_resolution::actual_captured;
		}

		switch (DownscaleQuality)
		{
			case EDolbyIOScreenshareDownscaleQuality::Medium:
				Ret.quality = screen_share_content_info::downscale_quality::medium;
				break;
			case EDolbyIOScreenshareDownscaleQuality::High:
				Ret.quality = screen_share_content_info::downscale_quality::high;
				break;
			case EDolbyIOScreenshareDownscaleQuality::Highest:
				Ret.quality = screen_share_content_info::downscale_quality::highest;
				break;
			default:
				Ret.quality = screen_share_content_info::downscale_quality::low;
		}

		return Ret;
	}

	log_level ToSdkLogLevel(EDolbyIOLogLevel Level)
	{
		switch (Level)
		{
			case EDolbyIOLogLevel::Error:
				return log_level::ERROR;
			case EDolbyIOLogLevel::Warning:
				return log_level::WARNING;
			case EDolbyIOLogLevel::Info:
				return log_level::INFO;
			case EDolbyIOLogLevel::Debug:
				return log_level::DEBUG;
			case EDolbyIOLogLevel::Verbose:
				return log_level::VERBOSE;
			default:
				return log_level::OFF;
		}
	}

	FSdkNativeDeviceID ToSdkNativeDeviceID(const FString& ID)
	{
#if PLATFORM_WINDOWS
		return ToStdString(ID);
#else
		unsigned Ret;
		TTypeFromString<unsigned>::FromString(Ret, *ID);
		return Ret;
#endif
	}

	FString ToUnrealDeviceID(const FSdkNativeDeviceID& ID)
	{
#if PLATFORM_WINDOWS
		return ToFString(ID);
#else
		return ToFString(std::to_string(ID));
#endif
	}

	FDolbyIOAudioDevice ToFDolbyIOAudioDevice(const audio_device& Device)
	{
		return FDolbyIOAudioDevice{ToFText(Device.name()), ToUnrealDeviceID(Device.native_id())};
	}

	FDolbyIOVideoDevice ToFDolbyIOVideoDevice(const camera_device& Device)
	{
		return FDolbyIOVideoDevice{ToFText(Device.display_name), ToFString(Device.unique_id)};
	}

	camera_device ToSdkVideoDevice(const FDolbyIOVideoDevice& VideoDevice)
	{
		return camera_device{ToStdString(VideoDevice.Name.ToString()), ToStdString(VideoDevice.UniqueID)};
	}
}
