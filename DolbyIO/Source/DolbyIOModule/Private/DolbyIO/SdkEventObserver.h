// Copyright 2022 Dolby Laboratories

#pragma once

#include "DolbyIO/Typedefs.h"

struct FDolbyIOParticipantInfo;

namespace DolbyIO
{
	class ISdkEventObserver
	{
	public:
		virtual ~ISdkEventObserver() = default;

		virtual void OnTokenNeededEvent() = 0;
		virtual void OnInitializedEvent() = 0;
		virtual void OnConnectedEvent(const FParticipantID&) = 0;
		virtual void OnDisconnectedEvent() = 0;
		virtual void OnParticipantAddedEvent(const FDolbyIOParticipantInfo&) = 0;
		virtual void OnParticipantLeftEvent(const FDolbyIOParticipantInfo&) = 0;
		virtual void OnActiveSpeakersChangedEvent(const FParticipantIDs&) = 0;
		virtual void OnAudioLevelsChangedEvent(const FAudioLevels&) = 0;
	};
}
