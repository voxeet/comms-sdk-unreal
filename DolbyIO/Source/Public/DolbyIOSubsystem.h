// Copyright 2023 Dolby Laboratories

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"

#include "DolbyIOParticipantInfo.h"

#include <memory>

#include "Engine/EngineTypes.h"

#include "DolbyIOSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSubsystemOnTokenNeededDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSubsystemOnInitializedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnConnectedDelegate, const FString&, LocalParticipantID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSubsystemOnDisconnectedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnParticipantAddedDelegate, const EDolbyIOParticipantStatus,
                                             Status, const FDolbyIOParticipantInfo&, ParticipantInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnParticipantUpdatedDelegate, const EDolbyIOParticipantStatus,
                                             Status, const FDolbyIOParticipantInfo&, ParticipantInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnVideoTrackAddedDelegate, const FString&, ParticipantID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnVideoTrackRemovedDelegate, const FString&, ParticipantID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnActiveSpeakersChangedDelegate, const TArray<FString>&,
                                            ActiveSpeakers);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSubsystemOnAudioLevelsChangedDelegate, const TArray<FString>&,
                                             ActiveSpeakers, const TArray<float>&, AudioLevels);

namespace dolbyio::comms
{
	enum class conference_status;
	class refresh_token;
	class sdk;
}

namespace DolbyIO
{
	class FVideoSink;
}

UCLASS(DisplayName = "Dolby.io Subsystem")
class DOLBYIO_API UDolbyIOSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Initializes or refreshes the client access token. Initializes the plugin unless already initialized.
	 *
	 * Successful initialization triggers the On Initialized event.
	 *
	 * For quick testing, you can manually obtain a token from the Dolby.io dashboard (https://dashboard.dolby.io) and
	 * paste it directly into the node or use the Get Dolby.io Token function.
	 *
	 * @param Token - The client access token.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetToken(const FString& Token);

	/** Connects to a conference.
	 *
	 * Triggers On Connected if successful.
	 *
	 * @param ConferenceName - The conference name. Must not be empty.
	 * @param UserName - The name of the participant.
	 * @param ExternalID - The external unique identifier that the customer's application can add to the participant
	 * while opening a session. If a participant uses the same external ID in conferences, the participant's ID also
	 * remains the same across all sessions.
	 * @param AvatarURL - The URL of the participant's avatar.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void Connect(const FString& ConferenceName = "unreal", const FString& UserName = "", const FString& ExternalID = "",
	             const FString& AvatarURL = "");

	/** Connects to a demo conference.
	 *
	 * The demo automatically brings in 3 invisible bots into the conference as a quick way to validate the connection
	 * to the service with audio functionality. The bots are placed at point {0, 0, 0}.
	 *
	 * Triggers On Connected if successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void DemoConference();

	/** Disconnects from the current conference.
	 *
	 * Triggers On Disconnected when complete.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void Disconnect();

	/** Sets the spatial environment scale.
	 *
	 * The larger the scale, the longer the distance at which the spatial audio
	 * attenuates. To get the best experience, the scale should be set separately for each level. The default value of
	 * "1.0" means that audio will fall completely silent at a distance of 10000 units (10000 cm/100 m).
	 *
	 * @param SpatialEnvironmentScale - The scale as a floating point number.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetSpatialEnvironmentScale(float SpatialEnvironmentScale = 1.0f);

	/** Mutes audio input. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void MuteInput();

	/** Unmutes audio input. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnmuteInput();

	/** Mutes audio output. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void MuteOutput();

	/** Unmutes audio output. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void UnmuteOutput();

	/** Enables video streaming from the primary webcam. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void EnableVideo();

	/** Disables video streaming from the primary webcam. */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void DisableVideo();

	/** Gets the texture to which video from a given participant is being rendered.
	 *
	 * @param ParticipantID - The participant's ID.
	 * @return The texture holding the participant's video frame or NULL if no such texture exists.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	class UTexture2D* GetTexture(const FString& ParticipantID);

	/** Updates the location of the listener for spatial audio purposes.
	 *
	 * Calling this function even once disables the default behavior, which is to automatically use the location of the
	 * first player controller.
	 *
	 * @param Location - The location of the listener.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetLocalPlayerLocation(const FVector& Location);

	/** Updates the rotation of the listener for spatial audio purposes.
	 *
	 * Calling this function even once disables the default behavior, which is to automatically use the rotation of the
	 * first player controller.
	 *
	 * @param Rotation - The rotation of the listener.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dolby.io Comms")
	void SetLocalPlayerRotation(const FRotator& Rotation);

	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnTokenNeededDelegate OnTokenNeeded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnInitializedDelegate OnInitialized;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnConnectedDelegate OnConnected;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnDisconnectedDelegate OnDisconnected;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnParticipantAddedDelegate OnParticipantAdded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnParticipantUpdatedDelegate OnParticipantUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnVideoTrackAddedDelegate OnVideoTrackAdded;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnVideoTrackRemovedDelegate OnVideoTrackRemoved;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnActiveSpeakersChangedDelegate OnActiveSpeakersChanged;
	UPROPERTY(BlueprintAssignable, Category = "Dolby.io Comms")
	FSubsystemOnAudioLevelsChangedDelegate OnAudioLevelsChanged;

private:
	void Initialize(FSubsystemCollectionBase&) override;
	void Deinitialize() override;

	bool CanConnect() const;
	bool IsConnected() const;

	void Initialize(const FString& Token);
	void UpdateStatus(dolbyio::comms::conference_status);

	void SetSpatialEnvironment();

	void ToggleInputMute();
	void ToggleOutputMute();
	void ToggleVideo();

	void SetLocationUsingFirstPlayer();
	void SetLocalPlayerLocationImpl(const FVector& Location);
	void SetRotationUsingFirstPlayer();
	void SetLocalPlayerRotationImpl(const FRotator& Rotation);

	template <class TDelegate, class... TArgs> void BroadcastEvent(TDelegate&, TArgs&&...);

	dolbyio::comms::conference_status ConferenceStatus;
	FString LocalParticipantID;

	TMap<FString, std::shared_ptr<DolbyIO::FVideoSink>> VideoSinks;
	TSharedPtr<dolbyio::comms::sdk> Sdk;
	TSharedPtr<dolbyio::comms::refresh_token> RefreshTokenCb;

	float SpatialEnvironmentScale = 1.0f;

	bool bIsInputMuted = false;
	bool bIsOutputMuted = false;
	bool bIsVideoEnabled = false;

	FTimerHandle LocationTimerHandle;
	FTimerHandle RotationTimerHandle;

	class FErrorHandler final
	{
	public:
		FErrorHandler(UDolbyIOSubsystem& DolbyIOSubsystem, int Line);

		void operator()(std::exception_ptr&& ExcPtr) const;
		void HandleError() const;

	private:
		void HandleError(TFunction<void()> Callee) const;
		void LogException(const FString& Type, const FString& What) const;

		UDolbyIOSubsystem& DolbyIOSubsystem;
		int Line;
	};
};
