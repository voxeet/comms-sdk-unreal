// Copyright 2022 Dolby Laboratories

#pragma once

#include "DolbyIoConference.h"

#include "DolbyIoSampleConference.generated.h"

/** Sample implementation of DolbyIoConference. */
UCLASS()
class DOLBYIOSAMPLECONFERENCEMODULE_API ADolbyIoSampleConference : public ADolbyIoConference
{
	GENERATED_BODY()

public:
	void OnNewListOfInputDevices_Implementation() override;
	void OnNewListOfOutputDevices_Implementation() override;
	void OnLocalParticipantChanged_Implementation() override;
	void OnNewListOfRemoteParticipants_Implementation() override;
	void OnNewListOfActiveSpeakers_Implementation() override;
	void OnNewAudioLevels_Implementation() override;
	void OnRefreshTokenNeeded_Implementation() override;

	void Disconnect();
	void MuteInput();
	void MuteOutput();
	void PreviousInputDevice();
	void NextInputDevice();
	void PreviousOutputDevice();
	void NextOutputDevice();

private:
	void BeginPlay() override;

	int GetCurrentInputDeviceIndex() const;
	int GetCurrentOutputDeviceIndex() const;
};
