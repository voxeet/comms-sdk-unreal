// Copyright 2022 Dolby Laboratories

#pragma once

#include "Internationalization/Text.h"

namespace Dolby
{
	using FMessage = FString;
	using FDeviceName = FText;
	using FDeviceNames = TArray<FDeviceName>;
	using FParticipant = FString;
	using FParticipants = TSet<FParticipant>;
	using FAudioLevels = TMap<FParticipant, float>;

	class ISdkStatusObserver
	{
	public:
		virtual ~ISdkStatusObserver() = default;

		virtual void OnStatusChanged(const FMessage&) = 0;

		virtual void OnNewListOfInputDevices(const FDeviceNames&) = 0;
		virtual void OnNewListOfOutputDevices(const FDeviceNames&) = 0;
		virtual void OnInputDeviceChanged(const FDeviceName&) = 0;
		virtual void OnOutputDeviceChanged(const FDeviceName&) = 0;

		virtual void OnLocalParticipantChanged(const FParticipant&) = 0;
		virtual void OnNewListOfRemoteParticipants(const FParticipants&) = 0;
		virtual void OnNewListOfActiveSpeakers(const FParticipants&) = 0;
		virtual void OnNewAudioLevels(const FAudioLevels&) = 0;

		virtual void OnRefreshTokenRequested() = 0;
	};
}
