// Copyright 2022 Dolby Laboratories

#pragma once

#include "DolbyIO/Typedefs.h"

namespace DolbyIO
{
	class ISdkEventObserver
	{
	public:
		virtual ~ISdkEventObserver() = default;

		virtual void OnTokenNeededEvent() = 0;
		virtual void OnInitializedEvent() = 0;
		virtual void OnConnectedEvent(const FParticipant&) = 0;
		virtual void OnDisconnectedEvent() = 0;
		virtual void OnRemoteParticipantsChangedEvent(const FParticipants&) = 0;
		virtual void OnActiveSpeakersChangedEvent(const FParticipants&) = 0;
		virtual void OnAudioLevelsChangedEvent(const FAudioLevels&) = 0;
	};
}
