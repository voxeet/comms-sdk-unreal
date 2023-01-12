// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOParticipantInfo.generated.h"

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
	 * across all sessions. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString ExternalID;

	/** The URL of the participant's avatar. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString AvatarURL;

	/** Indicates whether the participants is a listener. True means this is a participant who can receive audio and
	 * video streams, but cannot send any stream to a conference. False means this is a participant who can send and
	 * receive an audio and video stream during a conference.*/
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	bool bIsListener;

	/** A boolean that informs whether the participant is sending audio into conference. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	bool bIsSendingAudio;

	/** Indicates whether a remote participant is audible locally. This will always be false for the local participant.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	bool bIsAudibleLocally;
};
