// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOConnectionMode.h"
#include "DolbyIOScreenshareSource.h"
#include "DolbyIOSpatialAudioStyle.h"
#include "DolbyIOVideoTrack.h"

#include "Kismet/BlueprintAsyncActionBase.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "DolbyIOFunctions.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDolbyIOSetTokenOutputPin);

UCLASS()
class DOLBYIO_API UDolbyIOSetToken : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
	                  DisplayName = "Dolby.io Set Token"))
	static UDolbyIOSetToken* DolbyIOSetToken(const UObject* WorldContextObject, const FString& Token);

	UPROPERTY(BlueprintAssignable)
	FDolbyIOSetTokenOutputPin OnInitialized;

private:
	void Activate() override;

	UFUNCTION()
	void OnInitializedImpl();

	const UObject* WorldContextObject;
	FString Token;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDolbyIOConnectOutputPin, const FString&, LocalParticipantID,
                                             const FString&, ConferenceID);

UCLASS()
class DOLBYIO_API UDolbyIOConnect : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
	                  DisplayName = "Dolby.io Connect"))
	static UDolbyIOConnect* DolbyIOConnect(
	    const UObject* WorldContextObject, const FString& ConferenceName = "unreal", const FString& UserName = "",
	    const FString& ExternalID = "", const FString& AvatarURL = "",
	    EDolbyIOConnectionMode ConnectionMode = EDolbyIOConnectionMode::Active,
	    EDolbyIOSpatialAudioStyle SpatialAudioStyle = EDolbyIOSpatialAudioStyle::Shared);

	UPROPERTY(BlueprintAssignable)
	FDolbyIOConnectOutputPin OnConnected;

private:
	void Activate() override;

	UFUNCTION()
	void OnConnectedImpl(const FString& LocalParticipantID, const FString& ConferenceID);

	const UObject* WorldContextObject;
	FString ConferenceName;
	FString UserName;
	FString ExternalID;
	FString AvatarURL;
	EDolbyIOConnectionMode ConnectionMode;
	EDolbyIOSpatialAudioStyle SpatialAudioStyle;
};

UCLASS()
class DOLBYIO_API UDolbyIODemoConference : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
	                  DisplayName = "Dolby.io Demo Conference"))
	static UDolbyIODemoConference* DolbyIODemoConference(const UObject* WorldContextObject);

	UPROPERTY(BlueprintAssignable)
	FDolbyIOConnectOutputPin OnConnected;

private:
	void Activate() override;

	UFUNCTION()
	void OnConnectedImpl(const FString& LocalParticipantID, const FString& ConferenceID);

	const UObject* WorldContextObject;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDolbyIODisconnectOutputPin);

UCLASS()
class DOLBYIO_API UDolbyIODisconnect : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
	                  DisplayName = "Dolby.io Disconnect"))
	static UDolbyIODisconnect* DolbyIODisconnect(const UObject* WorldContextObject);

	UPROPERTY(BlueprintAssignable)
	FDolbyIODisconnectOutputPin OnDisconnected;

private:
	void Activate() override;

	UFUNCTION()
	void OnDisconnectedImpl();

	const UObject* WorldContextObject;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDolbyIOGetScreenshareSourcesOutputPin,
                                            const TArray<FDolbyIOScreenshareSource>&, Sources);

UCLASS()
class DOLBYIO_API UDolbyIOGetScreenshareSources : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
	                  DisplayName = "Dolby.io Get Screenshare Sources"))
	static UDolbyIOGetScreenshareSources* DolbyIOGetScreenshareSources(const UObject* WorldContextObject);

	UPROPERTY(BlueprintAssignable)
	FDolbyIOGetScreenshareSourcesOutputPin OnScreenshareSourcesReceived;

private:
	void Activate() override;

	UFUNCTION()
	void OnScreenshareSourcesReceivedImpl(const TArray<FDolbyIOScreenshareSource>& Sources);

	const UObject* WorldContextObject;
};

UCLASS()
class UDolbyIOBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Sets the spatial environment scale.
	 *
	 * The larger the scale, the longer the distance at which the spatial audio
	 * attenuates. To get the best experience, the scale should be set separately for each level. The default value of
	 * "1.0" means that audio will fall completely silent at a distance of 10000 units (10000 cm/100 m).
	 *
	 * @param SpatialEnvironmentScale - The scale as a floating point number.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Set Spatial Environment Scale"))
	static void SetSpatialEnvironmentScale(const UObject* WorldContextObject, float SpatialEnvironmentScale = 1.0f);

	/** Mutes audio input. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Mute Input"))
	static void MuteInput(const UObject* WorldContextObject);

	/** Unmutes audio input. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Unmute Input"))
	static void UnmuteInput(const UObject* WorldContextObject);

	/** Mutes audio output. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Mute Output"))
	static void MuteOutput(const UObject* WorldContextObject);

	/** Unmutes audio output. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Unmute Output"))
	static void UnmuteOutput(const UObject* WorldContextObject);

	/** Mutes a given participant for the local user. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Mute Participant"))
	static void MuteParticipant(const UObject* WorldContextObject, const FString& ParticipantID);

	/** Unmutes a given participant for the local user. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Unmute Participant"))
	static void UnmuteParticipant(const UObject* WorldContextObject, const FString& ParticipantID);

	/** Enables video streaming from the primary webcam. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Enable Video"))
	static void EnableVideo(const UObject* WorldContextObject);

	/** Disables video streaming from the primary webcam. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Disable Video"))
	static void DisableVideo(const UObject* WorldContextObject);

	/** Binds a dynamic material instance to hold the frames of the given video track. The plugin will update the
	 * material's texture parameter named "DolbyIO Frame" with the necessary data, therefore the material should have
	 * such a parameter to be usable. Automatically unbinds the material from all other tracks, but it is possible to
	 * bind multiple materials to the same track. Has no effect if the track does not exist at the moment the function
	 * is called, therefore it should usually be called as a response to the "On Video Track Added" event.
	 *
	 * @param Material - The dynamic material instance to bind.
	 * @param VideoTrackID - The ID of the video track.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Bind Material"))
	static void BindMaterial(const UObject* WorldContextObject, UMaterialInstanceDynamic* Material,
	                         const FString& VideoTrackID);

	/** Unbinds a dynamic material instance to no longer hold the video frames of the given video track. The plugin will
	 * no longer update the material's texture parameter named "DolbyIO Frame" with the necessary data.
	 *
	 * @param Material - The dynamic material instance to unbind.
	 * @param VideoTrackID - The ID of the video track.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Unbind Material"))
	static void UnbindMaterial(const UObject* WorldContextObject, UMaterialInstanceDynamic* Material,
	                           const FString& VideoTrackID);

	/** Gets the texture to which video from a given track is being rendered.
	 *
	 * @param VideoTrackID - The ID of the video track.
	 * @return The texture holding the video tracks's frame or NULL if no such texture exists.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Get Texture"))
	static class UTexture2D* GetTexture(const UObject* WorldContextObject, const FString& VideoTrackID);

	/** Starts screen sharing using a given source and content type. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Start Screenshare"))
	static void StartScreenshare(
	    const UObject* WorldContextObject, const FDolbyIOScreenshareSource& Source,
	    EDolbyIOScreenshareContentType ContentType = EDolbyIOScreenshareContentType::Unspecified);

	/** Stops screen sharing. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Stop Screenshare"))
	static void StopScreenshare(const UObject* WorldContextObject);

	/** Changes the screenshare content type if already sharing screen. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Change Screenshare Content Type"))
	static void ChangeScreenshareContentType(const UObject* WorldContextObject,
	                                         EDolbyIOScreenshareContentType ContentType);

	/** Updates the location of the listener for spatial audio purposes.
	 *
	 * Calling this function even once disables the default behavior, which is to automatically use the location of the
	 * first player controller.
	 *
	 * @param Location - The location of the listener.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Set Local Player Location"))
	static void SetLocalPlayerLocation(const UObject* WorldContextObject, const FVector& Location);

	/** Updates the rotation of the listener for spatial audio purposes.
	 *
	 * Calling this function even once disables the default behavior, which is to automatically use the rotation of the
	 * first player controller.
	 *
	 * @param Rotation - The rotation of the listener.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms",
	          Meta = (WorldContext = "WorldContextObject", DisplayName = "Dolby.io Set Local Player Rotation"))
	static void SetLocalPlayerRotation(const UObject* WorldContextObject, const FRotator& Rotation);
};
