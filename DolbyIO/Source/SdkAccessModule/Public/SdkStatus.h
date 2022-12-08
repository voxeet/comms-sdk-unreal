// Copyright 2022 Dolby Laboratories

#pragma once

#include "CommonTypes.h"

namespace dolbyio
{
	namespace comms
	{
		enum class conference_status;
	}
}

namespace Dolby
{
	class ISdkEventsObserver;
	using EConferenceStatus = dolbyio::comms::conference_status;

	class FSdkStatus final
	{
	public:
		using NotifyingMethod = void (FSdkStatus::*)(const FMessage&);

		void SetObserver(ISdkEventsObserver*);
		bool IsConnected() const;
		void SetMsg(const FMessage&);
		void SetStatus(EConferenceStatus);

	private:
		// notifying methods
		void LogMsg(const FMessage&);
		void NotifyObserver(const FMessage&);

		EConferenceStatus ConferenceStatus;
		NotifyingMethod UpdateStatus = &FSdkStatus::LogMsg;
		ISdkEventsObserver* Observer = nullptr;
	};
}
