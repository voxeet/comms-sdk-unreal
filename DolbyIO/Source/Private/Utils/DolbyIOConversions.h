// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOTypes.h"
#include "Utils/DolbyIOCppSdk.h"

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
	FString ToString(const dolbyio::comms::camera_device& Device);
	FString ToString(const dolbyio::comms::screen_share_source& Source);

	EDolbyIOParticipantStatus ToEDolbyIOParticipantStatus(std::optional<dolbyio::comms::participant_status> Status);
	FDolbyIOParticipantInfo ToFDolbyIOParticipantInfo(const dolbyio::comms::participant_info& Info);
	FDolbyIOVideoTrack ToFDolbyIOVideoTrack(const dolbyio::comms::video_track& Track);
	FDolbyIOVideoTrack ToFDolbyIOVideoTrack(
	    const dolbyio::comms::utils::participant_track_map::value_type& TrackMapItem);

	dolbyio::comms::spatial_audio_style ToSdkSpatialAudioStyle(EDolbyIOSpatialAudioStyle SpatialAudioStyle);
	dolbyio::comms::screen_share_content_info ToSdkContentInfo(EDolbyIOScreenshareEncoderHint EncoderHint,
	                                                           EDolbyIOScreenshareMaxResolution MaxResolution,
	                                                           EDolbyIOScreenshareDownscaleQuality DownscaleQuality);
	dolbyio::comms::noise_reduction ToSdkNoiseReduction(EDolbyIONoiseReduction NoiseReduction);
	dolbyio::comms::voice_font ToSdkVoiceFont(EDolbyIOVoiceFont VoiceFont);
	dolbyio::comms::audio_capture_mode::standard ToSdkAudioCaptureMode(EDolbyIONoiseReduction NoiseReduction,
	                                                                   EDolbyIOVoiceFont VoiceFont);
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

	FDolbyIOScreenshareSource ToFDolbyIOScreenshareSource(const dolbyio::comms::screen_share_source& Source);
	dolbyio::comms::screen_share_source ToSdkScreenshareSource(const FDolbyIOScreenshareSource& Source);
}
