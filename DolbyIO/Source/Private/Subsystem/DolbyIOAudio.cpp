// Copyright 2023 Dolby Laboratories

#include "DolbyIO.h"

#include "Utils/DolbyIOConversions.h"
#include "Utils/DolbyIOErrorHandler.h"
#include "Utils/DolbyIOLogging.h"

using namespace dolbyio::comms;
using namespace DolbyIO;

void UDolbyIOSubsystem::SetSpatialEnvironment()
{
	if (!IsConnectedAsActive() || !IsSpatialAudio())
	{
		return;
	}

	// The SDK spatial settings expect meters as the default unit of length.
	// Unreal uses centimeters for scale, so the plugin's scale of "1" is a scale of "100" for the SDK.
	const float SdkScale = SpatialEnvironmentScale * ScaleCenti;
	const spatial_scale Scale{SdkScale, SdkScale, SdkScale};
	const spatial_position Forward{1, 0, 0};
	const spatial_position Up{0, 0, 1};
	const spatial_position Right{0, 1, 0};
	Sdk->conference()
	    .set_spatial_environment(Scale, Forward, Up, Right)
	    .on_error(DLB_ERROR_HANDLER(OnSetSpatialEnvironmentScaleError));
}

void UDolbyIOSubsystem::SetSpatialEnvironmentScale(float Scale)
{
	DLB_UE_LOG("Setting spatial environment scale: %f", Scale);
	SpatialEnvironmentScale = Scale;
	SetSpatialEnvironment();
}

void UDolbyIOSubsystem::MuteInput()
{
	DLB_UE_LOG("Muting input");
	bIsInputMuted = true;
	ToggleInputMute();
}

void UDolbyIOSubsystem::UnmuteInput()
{
	DLB_UE_LOG("Unmuting input");
	bIsInputMuted = false;
	ToggleInputMute();
}

void UDolbyIOSubsystem::MuteOutput()
{
	DLB_UE_LOG("Muting output");
	bIsOutputMuted = true;
	ToggleOutputMute();
}

void UDolbyIOSubsystem::UnmuteOutput()
{
	DLB_UE_LOG("Unmuting output");
	bIsOutputMuted = false;
	ToggleOutputMute();
}

void UDolbyIOSubsystem::ToggleInputMute()
{
	if (IsConnectedAsActive())
	{
		Sdk->conference()
		    .mute(bIsInputMuted)
		    .on_error(DLB_ERROR_HANDLER(bIsInputMuted ? OnMuteInputError : OnUnmuteInputError));
	}
}

void UDolbyIOSubsystem::ToggleOutputMute()
{
	if (IsConnected() && ConnectionMode != EDolbyIOConnectionMode::ListenerRTS)
	{
		Sdk->conference()
		    .mute_output(bIsOutputMuted)
		    .on_error(DLB_ERROR_HANDLER(bIsOutputMuted ? OnMuteOutputError : OnUnmuteOutputError));
	}
}

void UDolbyIOSubsystem::MuteParticipant(const FString& ParticipantID)
{
	if (!IsConnected() || ParticipantID == LocalParticipantID)
	{
		return;
	}

	DLB_UE_LOG("Muting participant ID %s", *ParticipantID);
	Sdk->audio().remote().stop(ToStdString(ParticipantID)).on_error(DLB_ERROR_HANDLER(OnMuteParticipantError));
}

void UDolbyIOSubsystem::UnmuteParticipant(const FString& ParticipantID)
{
	if (!IsConnected() || ParticipantID == LocalParticipantID)
	{
		return;
	}

	DLB_UE_LOG("Unmuting participant ID %s", *ParticipantID);
	Sdk->audio().remote().start(ToStdString(ParticipantID)).on_error(DLB_ERROR_HANDLER(OnUnmuteParticipantError));
}

bool UDolbyIOSubsystem::IsSpatialAudio() const
{
	return SpatialAudioStyle != EDolbyIOSpatialAudioStyle::Disabled;
}

void UDolbyIOSubsystem::SetAudioCaptureMode(EDolbyIONoiseReduction NoiseReduction, EDolbyIOVoiceFont VoiceFont)
{
	if (!Sdk)
	{
		DLB_WARNING(OnSetAudioCaptureModeError, "Cannot set audio capture mode - not initialized");
		return;
	}

	DLB_UE_LOG("Setting audio capture mode to %s %s", *UEnum::GetValueAsString(NoiseReduction),
	           *UEnum::GetValueAsString(VoiceFont));
	Sdk->audio()
	    .local()
	    .set_capture_mode(ToSdkAudioCaptureMode(NoiseReduction, VoiceFont))
	    .on_error(DLB_ERROR_HANDLER(OnSetAudioCaptureModeError));
}
