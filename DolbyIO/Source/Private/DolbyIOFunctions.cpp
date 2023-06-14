// Copyright 2023 Dolby Laboratories

#include "DolbyIOFunctions.h"

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	UDolbyIOSubsystem* GetDolbyIOSubsystem(const UObject* WorldContextObject)
	{
		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
		{
			return GameInstance->GetSubsystem<UDolbyIOSubsystem>();
		}
		return nullptr;
	}
}

// --------------------------------------------------------------------------------------------------------------------

UDolbyIOSetToken* UDolbyIOSetToken::DolbyIOSetToken(const UObject* WorldContextObject, const FString& Token)
{
	UDolbyIOSetToken* Self = NewObject<UDolbyIOSetToken>();
	Self->WorldContextObject = WorldContextObject;
	Self->Token = Token;
	return Self;
}

void UDolbyIOSetToken::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnInitialized.AddDynamic(this, &UDolbyIOSetToken::OnInitializedImpl);
		DolbyIOSubsystem->SetToken(Token);
	}
}

void UDolbyIOSetToken::OnInitializedImpl()
{
	OnInitialized.Broadcast();

	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnInitialized.RemoveDynamic(this, &UDolbyIOSetToken::OnInitializedImpl);
	}
}

// --------------------------------------------------------------------------------------------------------------------

UDolbyIOConnect* UDolbyIOConnect::DolbyIOConnect(const UObject* WorldContextObject, const FString& ConferenceName,
                                                 const FString& UserName, const FString& ExternalID,
                                                 const FString& AvatarURL, EDolbyIOConnectionMode ConnectionMode,
                                                 EDolbyIOSpatialAudioStyle SpatialAudioStyle, int MaxVideoStreams,
                                                 EDolbyIOVideoForwardingStrategy VideoForwardingStrategy)
{
	UDolbyIOConnect* Self = NewObject<UDolbyIOConnect>();
	Self->WorldContextObject = WorldContextObject;
	Self->ConferenceName = ConferenceName;
	Self->UserName = UserName;
	Self->ExternalID = ExternalID;
	Self->AvatarURL = AvatarURL;
	Self->ConnectionMode = ConnectionMode;
	Self->SpatialAudioStyle = SpatialAudioStyle;
	Self->MaxVideoStreams = MaxVideoStreams;
	Self->VideoForwardingStrategy = VideoForwardingStrategy;
	return Self;
}

void UDolbyIOConnect::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnConnected.AddDynamic(this, &UDolbyIOConnect::OnConnectedImpl);
		DolbyIOSubsystem->Connect(ConferenceName, UserName, ExternalID, AvatarURL, ConnectionMode, SpatialAudioStyle,
		                          MaxVideoStreams, VideoForwardingStrategy);
	}
}

void UDolbyIOConnect::OnConnectedImpl(const FString& LocalParticipantID, const FString& ConferenceID)
{
	OnConnected.Broadcast(LocalParticipantID, ConferenceID);

	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnConnected.RemoveDynamic(this, &UDolbyIOConnect::OnConnectedImpl);
	}
}

// --------------------------------------------------------------------------------------------------------------------

UDolbyIODemoConference* UDolbyIODemoConference::DolbyIODemoConference(const UObject* WorldContextObject)
{
	UDolbyIODemoConference* Self = NewObject<UDolbyIODemoConference>();
	Self->WorldContextObject = WorldContextObject;
	return Self;
}

void UDolbyIODemoConference::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnConnected.AddDynamic(this, &UDolbyIODemoConference::OnConnectedImpl);
		DolbyIOSubsystem->DemoConference();
	}
}

void UDolbyIODemoConference::OnConnectedImpl(const FString& LocalParticipantID, const FString& ConferenceID)
{
	OnConnected.Broadcast(LocalParticipantID, ConferenceID);

	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnConnected.RemoveDynamic(this, &UDolbyIODemoConference::OnConnectedImpl);
	}
}

// --------------------------------------------------------------------------------------------------------------------

UDolbyIODisconnect* UDolbyIODisconnect::DolbyIODisconnect(const UObject* WorldContextObject)
{
	UDolbyIODisconnect* Self = NewObject<UDolbyIODisconnect>();
	Self->WorldContextObject = WorldContextObject;
	return Self;
}

void UDolbyIODisconnect::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnDisconnected.AddDynamic(this, &UDolbyIODisconnect::OnDisconnectedImpl);
		DolbyIOSubsystem->Disconnect();
	}
}

void UDolbyIODisconnect::OnDisconnectedImpl()
{
	OnDisconnected.Broadcast();

	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnDisconnected.RemoveDynamic(this, &UDolbyIODisconnect::OnDisconnectedImpl);
	}
}

// --------------------------------------------------------------------------------------------------------------------

UDolbyIOEnableVideo* UDolbyIOEnableVideo::DolbyIOEnableVideo(const UObject* WorldContextObject,
                                                             const FDolbyIOVideoDevice& VideoDevice)
{
	UDolbyIOEnableVideo* Self = NewObject<UDolbyIOEnableVideo>();
	Self->WorldContextObject = WorldContextObject;
	Self->VideoDevice = VideoDevice;
	return Self;
}

void UDolbyIOEnableVideo::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnVideoEnabled.AddDynamic(this, &UDolbyIOEnableVideo::OnVideoEnabledImpl);
		DolbyIOSubsystem->EnableVideo(VideoDevice);
	}
}

void UDolbyIOEnableVideo::OnVideoEnabledImpl(const FString& VideoTrackID)
{
	OnVideoEnabled.Broadcast(VideoTrackID);

	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnVideoEnabled.RemoveDynamic(this, &UDolbyIOEnableVideo::OnVideoEnabledImpl);
	}
}

// --------------------------------------------------------------------------------------------------------------------

UDolbyIODisableVideo* UDolbyIODisableVideo::DolbyIODisableVideo(const UObject* WorldContextObject)
{
	UDolbyIODisableVideo* Self = NewObject<UDolbyIODisableVideo>();
	Self->WorldContextObject = WorldContextObject;
	return Self;
}

void UDolbyIODisableVideo::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnVideoDisabled.AddDynamic(this, &UDolbyIODisableVideo::OnVideoDisabledImpl);
		DolbyIOSubsystem->DisableVideo();
	}
}

void UDolbyIODisableVideo::OnVideoDisabledImpl(const FString& VideoTrackID)
{
	OnVideoDisabled.Broadcast(VideoTrackID);

	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnVideoDisabled.RemoveDynamic(this, &UDolbyIODisableVideo::OnVideoDisabledImpl);
	}
}

// --------------------------------------------------------------------------------------------------------------------

UDolbyIOGetScreenshareSources* UDolbyIOGetScreenshareSources::DolbyIOGetScreenshareSources(
    const UObject* WorldContextObject)
{
	UDolbyIOGetScreenshareSources* Self = NewObject<UDolbyIOGetScreenshareSources>();
	Self->WorldContextObject = WorldContextObject;
	return Self;
}

void UDolbyIOGetScreenshareSources::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnScreenshareSourcesReceived.AddDynamic(
		    this, &UDolbyIOGetScreenshareSources::OnScreenshareSourcesReceivedImpl);
		DolbyIOSubsystem->GetScreenshareSources();
	}
}

void UDolbyIOGetScreenshareSources::OnScreenshareSourcesReceivedImpl(const TArray<FDolbyIOScreenshareSource>& Sources)
{
	OnScreenshareSourcesReceived.Broadcast(Sources);

	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnScreenshareSourcesReceived.RemoveDynamic(
		    this, &UDolbyIOGetScreenshareSources::OnScreenshareSourcesReceivedImpl);
	}
}

// --------------------------------------------------------------------------------------------------------------------

UDolbyIOStartScreenshare* UDolbyIOStartScreenshare::DolbyIOStartScreenshare(
    const UObject* WorldContextObject, const FDolbyIOScreenshareSource& Source,
    EDolbyIOScreenshareEncoderHint EncoderHint, EDolbyIOScreenshareMaxResolution MaxResolution,
    EDolbyIOScreenshareDownscaleQuality DownscaleQuality)
{
	UDolbyIOStartScreenshare* Self = NewObject<UDolbyIOStartScreenshare>();
	Self->WorldContextObject = WorldContextObject;
	Self->Source = Source;
	Self->EncoderHint = EncoderHint;
	Self->MaxResolution = MaxResolution;
	Self->DownscaleQuality = DownscaleQuality;
	return Self;
}

void UDolbyIOStartScreenshare::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnScreenshareStarted.AddDynamic(this, &UDolbyIOStartScreenshare::OnScreenshareStartedImpl);
		DolbyIOSubsystem->StartScreenshare(Source, EncoderHint, MaxResolution, DownscaleQuality);
	}
}

void UDolbyIOStartScreenshare::OnScreenshareStartedImpl(const FString& VideoTrackID)
{
	OnScreenshareStarted.Broadcast(VideoTrackID);

	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnScreenshareStarted.RemoveDynamic(this, &UDolbyIOStartScreenshare::OnScreenshareStartedImpl);
	}
}

// --------------------------------------------------------------------------------------------------------------------

UDolbyIOStopScreenshare* UDolbyIOStopScreenshare::DolbyIOStopScreenshare(const UObject* WorldContextObject)
{
	UDolbyIOStopScreenshare* Self = NewObject<UDolbyIOStopScreenshare>();
	Self->WorldContextObject = WorldContextObject;
	return Self;
}

void UDolbyIOStopScreenshare::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnScreenshareStopped.AddDynamic(this, &UDolbyIOStopScreenshare::OnScreenshareStoppedImpl);
		DolbyIOSubsystem->StopScreenshare();
	}
}

void UDolbyIOStopScreenshare::OnScreenshareStoppedImpl(const FString& VideoTrackID)
{
	OnScreenshareStopped.Broadcast(VideoTrackID);

	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnScreenshareStopped.RemoveDynamic(this, &UDolbyIOStopScreenshare::OnScreenshareStoppedImpl);
	}
}

// --------------------------------------------------------------------------------------------------------------------

UDolbyIOGetAudioInputDevices* UDolbyIOGetAudioInputDevices::DolbyIOGetAudioInputDevices(
    const UObject* WorldContextObject)
{
	UDolbyIOGetAudioInputDevices* Self = NewObject<UDolbyIOGetAudioInputDevices>();
	Self->WorldContextObject = WorldContextObject;
	return Self;
}

void UDolbyIOGetAudioInputDevices::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnAudioInputDevicesReceived.AddDynamic(
		    this, &UDolbyIOGetAudioInputDevices::OnAudioInputDevicesReceivedImpl);
		DolbyIOSubsystem->GetAudioInputDevices();
	}
}

void UDolbyIOGetAudioInputDevices::OnAudioInputDevicesReceivedImpl(const TArray<FDolbyIOAudioDevice>& Devices)
{
	OnAudioInputDevicesReceived.Broadcast(Devices);

	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnAudioInputDevicesReceived.RemoveDynamic(
		    this, &UDolbyIOGetAudioInputDevices::OnAudioInputDevicesReceivedImpl);
	}
}

// --------------------------------------------------------------------------------------------------------------------

UDolbyIOGetAudioOutputDevices* UDolbyIOGetAudioOutputDevices::DolbyIOGetAudioOutputDevices(
    const UObject* WorldContextObject)
{
	UDolbyIOGetAudioOutputDevices* Self = NewObject<UDolbyIOGetAudioOutputDevices>();
	Self->WorldContextObject = WorldContextObject;
	return Self;
}

void UDolbyIOGetAudioOutputDevices::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnAudioOutputDevicesReceived.AddDynamic(
		    this, &UDolbyIOGetAudioOutputDevices::OnAudioOutputDevicesReceivedImpl);
		DolbyIOSubsystem->GetAudioOutputDevices();
	}
}

void UDolbyIOGetAudioOutputDevices::OnAudioOutputDevicesReceivedImpl(const TArray<FDolbyIOAudioDevice>& Devices)
{
	OnAudioOutputDevicesReceived.Broadcast(Devices);

	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnAudioOutputDevicesReceived.RemoveDynamic(
		    this, &UDolbyIOGetAudioOutputDevices::OnAudioOutputDevicesReceivedImpl);
	}
}

// --------------------------------------------------------------------------------------------------------------------

UDolbyIOGetCurrentAudioInputDevice* UDolbyIOGetCurrentAudioInputDevice::DolbyIOGetCurrentAudioInputDevice(
    const UObject* WorldContextObject)
{
	UDolbyIOGetCurrentAudioInputDevice* Self = NewObject<UDolbyIOGetCurrentAudioInputDevice>();
	Self->WorldContextObject = WorldContextObject;
	return Self;
}

void UDolbyIOGetCurrentAudioInputDevice::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnCurrentAudioInputDeviceReceived.AddDynamic(
		    this, &UDolbyIOGetCurrentAudioInputDevice::OnCurrentAudioInputDeviceReceivedImpl);
		DolbyIOSubsystem->GetCurrentAudioInputDevice();
	}
}

void UDolbyIOGetCurrentAudioInputDevice::OnCurrentAudioInputDeviceReceivedImpl(
    bool IsNone, const FDolbyIOAudioDevice& OptionalDevice)
{
	OnCurrentAudioInputDeviceReceived.Broadcast(IsNone, OptionalDevice);

	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnCurrentAudioInputDeviceReceived.RemoveDynamic(
		    this, &UDolbyIOGetCurrentAudioInputDevice::OnCurrentAudioInputDeviceReceivedImpl);
	}
}

// --------------------------------------------------------------------------------------------------------------------

UDolbyIOGetCurrentAudioOutputDevice* UDolbyIOGetCurrentAudioOutputDevice::DolbyIOGetCurrentAudioOutputDevice(
    const UObject* WorldContextObject)
{
	UDolbyIOGetCurrentAudioOutputDevice* Self = NewObject<UDolbyIOGetCurrentAudioOutputDevice>();
	Self->WorldContextObject = WorldContextObject;
	return Self;
}

void UDolbyIOGetCurrentAudioOutputDevice::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnCurrentAudioOutputDeviceReceived.AddDynamic(
		    this, &UDolbyIOGetCurrentAudioOutputDevice::OnCurrentAudioOutputDeviceReceivedImpl);
		DolbyIOSubsystem->GetCurrentAudioOutputDevice();
	}
}

void UDolbyIOGetCurrentAudioOutputDevice::OnCurrentAudioOutputDeviceReceivedImpl(
    bool IsNone, const FDolbyIOAudioDevice& OptionalDevice)
{
	OnCurrentAudioOutputDeviceReceived.Broadcast(IsNone, OptionalDevice);

	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnCurrentAudioOutputDeviceReceived.RemoveDynamic(
		    this, &UDolbyIOGetCurrentAudioOutputDevice::OnCurrentAudioOutputDeviceReceivedImpl);
	}
}

// --------------------------------------------------------------------------------------------------------------------

UDolbyIOGetVideoDevices* UDolbyIOGetVideoDevices::DolbyIOGetVideoDevices(const UObject* WorldContextObject)
{
	UDolbyIOGetVideoDevices* Self = NewObject<UDolbyIOGetVideoDevices>();
	Self->WorldContextObject = WorldContextObject;
	return Self;
}

void UDolbyIOGetVideoDevices::Activate()
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnVideoDevicesReceived.AddDynamic(this, &UDolbyIOGetVideoDevices::OnVideoDevicesReceivedImpl);
		DolbyIOSubsystem->GetVideoDevices();
	}
}

void UDolbyIOGetVideoDevices::OnVideoDevicesReceivedImpl(const TArray<FDolbyIOVideoDevice>& Devices)
{
	OnVideoDevicesReceived.Broadcast(Devices);

	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->OnVideoDevicesReceived.RemoveDynamic(this,
		                                                       &UDolbyIOGetVideoDevices::OnVideoDevicesReceivedImpl);
	}
}

// --------------------------------------------------------------------------------------------------------------------

void UDolbyIOBlueprintFunctionLibrary::SetSpatialEnvironmentScale(const UObject* WorldContextObject,
                                                                  float SpatialEnvironmentScale)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->SetSpatialEnvironmentScale(SpatialEnvironmentScale);
	}
}
void UDolbyIOBlueprintFunctionLibrary::MuteInput(const UObject* WorldContextObject)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->MuteInput();
	}
}
void UDolbyIOBlueprintFunctionLibrary::UnmuteInput(const UObject* WorldContextObject)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->UnmuteInput();
	}
}
void UDolbyIOBlueprintFunctionLibrary::MuteOutput(const UObject* WorldContextObject)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->MuteOutput();
	}
}
void UDolbyIOBlueprintFunctionLibrary::UnmuteOutput(const UObject* WorldContextObject)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->UnmuteOutput();
	}
}
void UDolbyIOBlueprintFunctionLibrary::MuteParticipant(const UObject* WorldContextObject, const FString& ParticipantID)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->MuteParticipant(ParticipantID);
	}
}
void UDolbyIOBlueprintFunctionLibrary::UnmuteParticipant(const UObject* WorldContextObject,
                                                         const FString& ParticipantID)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->UnmuteParticipant(ParticipantID);
	}
}
TArray<FDolbyIOParticipantInfo> UDolbyIOBlueprintFunctionLibrary::GetParticipants(const UObject* WorldContextObject)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		return DolbyIOSubsystem->GetParticipants();
	}
	return {};
}
void UDolbyIOBlueprintFunctionLibrary::BindMaterial(const UObject* WorldContextObject,
                                                    UMaterialInstanceDynamic* Material, const FString& VideoTrackID)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->BindMaterial(Material, VideoTrackID);
	}
}
void UDolbyIOBlueprintFunctionLibrary::UnbindMaterial(const UObject* WorldContextObject,
                                                      UMaterialInstanceDynamic* Material, const FString& VideoTrackID)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->UnbindMaterial(Material, VideoTrackID);
	}
}
UTexture2D* UDolbyIOBlueprintFunctionLibrary::GetTexture(const UObject* WorldContextObject, const FString& VideoTrackID)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		return DolbyIOSubsystem->GetTexture(VideoTrackID);
	}
	return nullptr;
}
void UDolbyIOBlueprintFunctionLibrary::ChangeScreenshareParameters(const UObject* WorldContextObject,
                                                                   EDolbyIOScreenshareEncoderHint EncoderHint,
                                                                   EDolbyIOScreenshareMaxResolution MaxResolution,
                                                                   EDolbyIOScreenshareDownscaleQuality DownscaleQuality)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->ChangeScreenshareParameters(EncoderHint, MaxResolution, DownscaleQuality);
	}
}
void UDolbyIOBlueprintFunctionLibrary::SetLocalPlayerLocation(const UObject* WorldContextObject,
                                                              const FVector& Location)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->SetLocalPlayerLocation(Location);
	}
}
void UDolbyIOBlueprintFunctionLibrary::SetLocalPlayerRotation(const UObject* WorldContextObject,
                                                              const FRotator& Rotation)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->SetLocalPlayerRotation(Rotation);
	}
}
void UDolbyIOBlueprintFunctionLibrary::SetRemotePlayerLocation(const UObject* WorldContextObject,
                                                               const FString& ParticipantID, const FVector& Location)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->SetRemotePlayerLocation(ParticipantID, Location);
	}
}
void UDolbyIOBlueprintFunctionLibrary::SetLogSettings(const UObject* WorldContextObject, EDolbyIOLogLevel SdkLogLevel,
                                                      EDolbyIOLogLevel MediaLogLevel, EDolbyIOLogLevel DvcLogLevel)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->SetLogSettings(SdkLogLevel, MediaLogLevel, DvcLogLevel);
	}
}

void UDolbyIOBlueprintFunctionLibrary::SetAudioInputDevice(const UObject* WorldContextObject, const FString& NativeID)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->SetAudioInputDevice(NativeID);
	}
}

void UDolbyIOBlueprintFunctionLibrary::SetAudioOutputDevice(const UObject* WorldContextObject, const FString& NativeID)
{
	if (UDolbyIOSubsystem* DolbyIOSubsystem = GetDolbyIOSubsystem(WorldContextObject))
	{
		DolbyIOSubsystem->SetAudioOutputDevice(NativeID);
	}
}
