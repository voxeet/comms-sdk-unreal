// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOConnectionMode.h"
#include "DolbyIOCppSdk.h"
#include "DolbyIODevices.h"
#include "DolbyIOLogLevel.h"
#include "DolbyIOParticipantInfo.h"
#include "DolbyIOScreenshareSource.h"
#include "DolbyIOSpatialAudioStyle.h"
#include "DolbyIOVideoTrack.h"

#include <dolbyio/comms/utils/vfs_event.h>

namespace DolbyIO
{
	constexpr int ScaleCenti = 100;

	std::string ToStdString(const FString& String);
	FString ToFString(const std::string& String);
	FText ToFText(const std::string& String);

	FString ToString(dolbyio::comms::conference_status Status);
	FString ToString(dolbyio::comms::participant_status Status);
	FString ToString(const dolbyio::comms::audio_device& Device);
	FString ToString(enum dolbyio::comms::audio_device::direction Direction);

	EDolbyIOParticipantStatus ToEDolbyIOParticipantStatus(std::optional<dolbyio::comms::participant_status> Status);
	FDolbyIOParticipantInfo ToFDolbyIOParticipantInfo(const dolbyio::comms::participant_info& Info);
	FDolbyIOVideoTrack ToFDolbyIOVideoTrack(const dolbyio::comms::video_track& Track);
	FDolbyIOVideoTrack ToFDolbyIOVideoTrack(
	    const dolbyio::comms::utils::participant_track_map::value_type& TrackMapItem);

	dolbyio::comms::spatial_audio_style ToSdkSpatialAudioStyle(EDolbyIOSpatialAudioStyle SpatialAudioStyle);
	dolbyio::comms::screen_share_content_info ToSdkContentInfo(EDolbyIOScreenshareEncoderHint EncoderHint,
	                                                           EDolbyIOScreenshareMaxResolution MaxResolution,
	                                                           EDolbyIOScreenshareDownscaleQuality DownscaleQuality);
	dolbyio::comms::log_level ToSdkLogLevel(EDolbyIOLogLevel Level);
	dolbyio::comms::camera_device ToSdkVideoDevice(const FDolbyIOVideoDevice& VideoDevice);

	using FSdkNativeDeviceID =
#if PLATFORM_WINDOWS
	    std::string;
#else
	    unsigned;
#endif
	FSdkNativeDeviceID ToSdkNativeDeviceID(const FString& ID);
	FString ToUnrealDeviceID(const FSdkNativeDeviceID& ID);

	FDolbyIOAudioDevice ToFDolbyIOAudioDevice(const dolbyio::comms::audio_device& Device);
	FDolbyIOVideoDevice ToFDolbyIOVideoDevice(const dolbyio::comms::camera_device& Device);
}
