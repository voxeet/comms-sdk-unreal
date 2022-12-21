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

		virtual void OnListOfInputDevicesChanged(const FDeviceNames) = 0;
		virtual void OnListOfOutputDevicesChanged(const FDeviceNames) = 0;
		virtual void OnInputDeviceChanged(const int Index) = 0;
		virtual void OnOutputDeviceChanged(const int Index) = 0;

		virtual void OnLocalParticipantChanged(const FParticipant&) = 0;
		virtual void OnListOfRemoteParticipantsChanged(const FParticipants&) = 0;
		virtual void OnListOfActiveSpeakersChanged(const FParticipants) = 0;
		virtual void OnAudioLevelsChanged(const Dolby::FAudioLevels Levels) = 0;

		virtual void OnRefreshTokenRequested() = 0;
	};
}
