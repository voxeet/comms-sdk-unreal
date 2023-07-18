// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIOTypes.generated.h"

/** Defines whether to connect as an active user or a listener. */
UENUM(BlueprintType, DisplayName = "Dolby.io Connection Mode")
enum class EDolbyIOConnectionMode : uint8
{
	/** Join a conference as an active user. */
	Active,
	/** Receive multiple video streams from senders. */
	ListenerRegular,
	/** Receive a realtime mixed stream. */
	ListenerRTS,
};

/** The possible spatial audio styles of the conference. */
UENUM(BlueprintType, DisplayName = "Dolby.io Spatial Audio Style")
enum class EDolbyIOSpatialAudioStyle : uint8
{
	/** Spatial audio is disabled. */
	Disabled,
	/** The individual option sets the spatial location that is based on the spatial scene, local participant's
	 * position, and remote participants' positions. This allows a client to control the position using the local,
	 * self-contained logic. However, the client has to communicate a large set of requests constantly to the server,
	 * which increases network traffic, log subsystem pressure, and complexity of the client-side application.
	 */
	Individual,
	/** Spatial audio for shared scenes, The shared option sets the spatial location that is based on the spatial scene
	 * and the local participant's position, while the relative positions among participants are calculated by the
	 * Dolby.io server. This way, the spatial scene is shared by all participants, so that each client can set its own
	 * position and participate in the shared scene. This approach simplifies communication between the client and the
	 * server and decreases network traffic.
	 */
	Shared,
};

/** Defines how the plugin should select conference participants whose videos will be transmitted to the local
 * participant.
 */
UENUM(BlueprintType, DisplayName = "Dolby.io Video Forwarding Strategy")
enum class EDolbyIOVideoForwardingStrategy : uint8
{
	/** Selects participants based on their audio volume.
	 */
	LastSpeaker,
	/** Selects participants based on the distance from the local participant. This strategy is available only for
	 * participants who enabled spatial audio.
	 */
	ClosestUser
};

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

	/** The conference status of the remote participant. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	EDolbyIOParticipantStatus Status{};
};

/** The platform-agnostic description of an audio device. */
USTRUCT(BlueprintType, DisplayName = "Dolby.io Audio Device")
struct DOLBYIO_API FDolbyIOAudioDevice
{
	GENERATED_BODY()

	/** The name of the device. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FText Name;

	/** The native ID of the device. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString NativeID;
};

/** The platform agnostic description of a video device. */
USTRUCT(BlueprintType, DisplayName = "Dolby.io Video Device")
struct DOLBYIO_API FDolbyIOVideoDevice
{
	GENERATED_BODY()

	/** The name of the device. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FText Name;

	/** The unique ID of the device. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString UniqueID;
};

/** The platform agnostic description of source for screen sharing. */
USTRUCT(BlueprintType, DisplayName = "Dolby.io Screenshare Source")
struct DOLBYIO_API FDolbyIOScreenshareSource
{
	GENERATED_BODY()

	/** Unique id of the screen in question. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	int64 ID{};

	/** Indicates whether this is an entire screen (true) or an application window (false). */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	bool bIsScreen{};

	/** Title of the screen. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FText Title;
};

/** The encoder hint provides a hint to the plugin as to what type of content is being captured by the screen share.
 * This can be used to help when making decisions for choosing encoder settings based on what aspects of the content are
 * important. For instance, if network conditions worsen is it more important to keep a higher resolution of screen
 * share or frame rate to keep the motion more fluid.
 */
UENUM(BlueprintType, DisplayName = "Dolby.io Screenshare Encoder Hint")
enum class EDolbyIOScreenshareEncoderHint : uint8
{
	/** The content hint is not specified, encoder will choose settings based on the fact track is screen share only. */
	Unspecified,
	/** Content is detailed, encoder should keep resolution high if it can. */
	Detailed,
	/** Content is text, encoder should keep resolution high if it can. */
	Text,
	/** Content is fluid, full of motion, encoder should keep frame rate higher. */
	Fluid,
};

/** The max resolution for the capture screen content to be shared as. If the actual captured resolution is higher than
 * the set max resolution the plugin will downscale to the max resolution. The plugin captures screen content based on
 * the chosen display's resolution. The max resolution will also be downscaled if the captured resolution is higher,
 * otherwise this has no effect. Therefore the screen content will never be shared at a resolution higher than the one
 * set here, but if the captured display has lower resolution the content will not be upscaled.
 */
UENUM(BlueprintType, DisplayName = "Dolby.io Screenshare Max Resolution")
enum class EDolbyIOScreenshareMaxResolution : uint8
{
	/** Downscale the height to 1080p, the width will be chosen to respect the actual aspect ratio of the raw captured
	 * frame. Downscaling will only happen if the captured resolution is higher. */
	DownscaleTo1080p,
	/** Downscale the height to 1440p, the width will be chosen to respect the actual aspect ratio of the raw captured
	 * frame. Downscaling will only happen if the captured resolution is higher. */
	DownscaleTo1440p,
	/** No downscaling, send the content to the encoder in the actual captured resolution. */
	ActualCaptured,
};

/** The quality for the downscaling algorithm to be used. The higher the quality, the clearer the picture will be, but
 * the higher the CPU usage will be.
 */
UENUM(BlueprintType, DisplayName = "Dolby.io Screenshare Downscale Quality")
enum class EDolbyIOScreenshareDownscaleQuality : uint8
{
	/** Use the quicker algorithm, fast and lowest CPU used but low quality. */
	Low,
	/** Use the linear filter algorithm used mainly for horizontal scaling. */
	Medium,
	/** Use the bilinear filter algorithm, faster than highest, but quality not as good. If downscaling fluid screen
	 * content from 4K to 1080p this option should be used.
	 */
	High,
	/** Use the box filter algorithm, uses the most CPU and is the slowest but produces the best quality. If downscaling
	 * detailed screen content from 4K to 2K this option should be used.
	 */
	Highest,
};

/** Contains data about a Dolby.io video track. */
USTRUCT(BlueprintType, DisplayName = "Dolby.io Video Track")
struct DOLBYIO_API FDolbyIOVideoTrack
{
	GENERATED_BODY()

	/** The unique ID of the video track. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString TrackID;

	/** The participant from whom the track is coming. */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	FString ParticipantID;

	/** Indicates whether this is a screenshare video track (true) or a camera video track (false). */
	UPROPERTY(BlueprintReadOnly, Category = "Dolby.io Comms")
	bool bIsScreenshare{};
};

/** The level of logs of the Dolby.io C++ SDK. */
UENUM(BlueprintType, DisplayName = "Dolby.io Log Level")
enum class EDolbyIOLogLevel : uint8
{
	/** Turns off logging. */
	Off,

	/** The error level logging generates logs when an error occurs and the SDK cannot properly function. */
	Error,

	/** The warning level logging generates logs when the SDK detects an unexpected problem but is still able to work as
	 * usual.
	 */
	Warning,

	/** The info level logging generates an informative number of logs. */
	Info,

	/** The debug level logging generates a high number of logs to provide diagnostic information in a detailed manner.
	 */
	Debug,

	/** The verbose level logging generates the highest number of logs, including even the HTTP requests. */
	Verbose
};

/** The audio noise reduction level. */
UENUM(BlueprintType, DisplayName = "Dolby.io Noise Reduction")
enum class EDolbyIONoiseReduction : uint8
{
	/** Reduce all noise and isolate voice. */
	High,

	/** Remove steady/stationary background noise. */
	Low
};

/** Voice modification effect. */
UENUM(BlueprintType, DisplayName = "Dolby.io Voice Font")
enum class EDolbyIOVoiceFont : uint8
{
	None,
	Masculine,
	Feminine,
	Helium,
	DarkModulation,
	BrokenRobot,
	Interference,
	Abyss,
	Wobble,
	StarshipCaptain,
	NervousRobot,
	Swarm,
	AMRadio
};
