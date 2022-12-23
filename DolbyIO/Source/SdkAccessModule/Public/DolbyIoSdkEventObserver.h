// Copyright 2022 Dolby Laboratories

#pragma once

#include "DolbyIoTypedefs.h"

namespace Dolby
{
	class ISdkEventObserver
	{
	public:
		virtual ~ISdkEventObserver() = default;

		virtual void OnTokenNeededEvent() = 0;
		virtual void OnInitializedEvent() = 0;
		virtual void OnConnectedEvent() = 0;
		virtual void OnDisconnectedEvent() = 0;

		virtual void OnLocalParticipantChangedEvent(const FParticipant&) = 0;
		virtual void OnListOfRemoteParticipantsChangedEvent(const FParticipants&) = 0;
		virtual void OnListOfActiveSpeakersChangedEvent(const FParticipants&) = 0;
		virtual void OnListOfAudioLevelsChangedEvent(const FAudioLevels&) = 0;

		virtual void OnListOfInputDevicesChangedEvent(const FDeviceNames&) = 0;
		virtual void OnListOfOutputDevicesChangedEvent(const FDeviceNames&) = 0;
		virtual void OnCurrentInputDeviceChangedEvent(int Index) = 0;
		virtual void OnCurrentOutputDeviceChangedEvent(int Index) = 0;
	};
}
