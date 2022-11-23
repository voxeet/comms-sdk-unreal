// Copyright 2022 Dolby Laboratories

#pragma once

#include "CommonTypes.h"

namespace Dolby
{
	class ISdkEventsObserver
	{
	public:
		virtual ~ISdkEventsObserver() = default;

		virtual void OnStatusChanged(const FMessage&) = 0;

		virtual void OnListOfInputDevicesChanged() = 0;
		virtual void OnListOfOutputDevicesChanged() = 0;
		virtual void OnInputDeviceChanged(int Index) = 0;
		virtual void OnOutputDeviceChanged(int Index) = 0;

		virtual void OnLocalParticipantChanged(const FParticipant&) = 0;
		virtual void OnNewListOfRemoteParticipants(const FParticipants&) = 0;
		virtual void OnNewListOfActiveSpeakers(const FParticipants&) = 0;
		virtual void OnNewAudioLevels(const FAudioLevels&) = 0;

		virtual void OnRefreshTokenRequested() = 0;
	};
}
