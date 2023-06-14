// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOParticipantInfo.generated.h"

UENUM(BlueprintType, DisplayName = "Dolby.io Participant Status")
enum class EDolbyIOParticipantStatus : uint8
{
	/** A participant is invited to a conference. */
	Reserved,
	/** A participant received the conference invitation and is connecting to a conference. */
	Connecting,
	/** A participant successfully connected to a conference. */
	OnAir,
	/** An invited participant declined the conference invitation. Currently, the Web SDK does not offer the decline
	 * method, although participants who use Web SDK can receive the Decline status from other application users who use
	 * the Android or iOS SDK. The Web SDK does not receive the notification if the invited participant uses the decline
	 * method after joining a conference.
	 */
	Decline,
	/** A participant left the conference. */
	Left,
	/** A participant did not enable audio, video, or screen-share and, therefore, is not connected to any stream. */
	Inactive,
	/** A participant experiences a peer connection problem, which may result in the Error or Connected status. */
	Warning,
	/** A peer connection failed, and the participant cannot connect to a conference. */
	Error,
	/** A participant was kicked from the conference. */
	Kicked,
	Unknown,
};

/** Contains the current status of a conference participant and information whether the participant's audio is enabled.
 */
USTRUCT(BlueprintType, DisplayName = "Dolby.io Participant Info")
struct DOLBYIO_API FDolbyIOParticipantInfo
{
	GENERATED_BODY()

	/** The unique identifier of the participant. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString UserID;

	/** The participant's name. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString Name;

	/** The external unique identifier that the customer's application can add to the participant while opening a
	 * session. If a participant uses the same external ID in conferences, the participant's ID also remains the same
	 * across all sessions.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString ExternalID;

	/** The URL of the participant's avatar. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString AvatarURL;

	/** Indicates whether the participants is a listener. True means this is a participant who can receive audio and
	 * video streams, but cannot send any stream to a conference. False means this is a participant who can send and
	 * receive an audio and video stream during a conference.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	bool bIsListener{};

	/** A boolean that informs whether the participant is sending audio into conference. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	bool bIsSendingAudio{};

	/** Indicates whether a remote participant is audible locally. This will always be false for the local participant.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	bool bIsAudibleLocally{};

	/** A boolean that informs whether the participant is an injected bot. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	bool bIsInjectedBot{};

	/** The initial location of the participant. Meaningless unless the participant is an injected bot. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FVector Location{0, 0, 0};

	/** The initial rotation of the participant. Meaningless unless the participant is an injected bot. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FRotator Rotation{0, 0, 0};

	/** The conference status of the remote participant. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	EDolbyIOParticipantStatus Status{};
};
