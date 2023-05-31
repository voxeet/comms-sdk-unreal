// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOConnectionMode.h"
#include "DolbyIOCppSdk.h"
#include "DolbyIOLogLevel.h"
#include "DolbyIOParticipantInfo.h"
#include "DolbyIOScreenshareSource.h"
#include "DolbyIOSpatialAudioStyle.h"
#include "DolbyIOVideoTrack.h"

namespace DolbyIO
{
	constexpr int ScaleCenti = 100;

	std::string ToStdString(const FString& String);
	FString ToFString(const std::string& String);
	FText ToFText(const std::string& String);

	FString ToString(dolbyio::comms::conference_status Status);
	FString ToString(dolbyio::comms::participant_status Status);
	FString ToString(EDolbyIOConnectionMode ConnectionMode);
	FString ToString(EDolbyIOSpatialAudioStyle SpatialAudioStyle);

	EDolbyIOParticipantStatus ToEDolbyIOParticipantStatus(std::optional<dolbyio::comms::participant_status> Status);
	FDolbyIOParticipantInfo ToFDolbyIOParticipantInfo(const dolbyio::comms::participant_info& Info);
	FDolbyIOVideoTrack ToFDolbyIOVideoTrack(const dolbyio::comms::video_track& Track);

	dolbyio::comms::spatial_audio_style ToSdkSpatialAudioStyle(EDolbyIOSpatialAudioStyle SpatialAudioStyle);
	dolbyio::comms::screen_share_content_type ToSdkContentType(EDolbyIOScreenshareContentType Type);
	dolbyio::comms::log_level ToSdkLogLevel(EDolbyIOLogLevel Level);
}
