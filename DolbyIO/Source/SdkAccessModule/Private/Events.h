// Copyright 2022 Dolby Laboratories

#pragma once

#include <dolbyio/comms/event_handling.h>

#include "Containers/Array.h"

namespace dolbyio::comms
{
	class event_handler_connection;
}

namespace Dolby
{
	class FSdkStatus;

	class FEvents final
	{
		using EventHandlerId = dolbyio::comms::event_handler_id;

	public:
		FEvents(FSdkStatus&);
		~FEvents();

		void AddEvent(EventHandlerId);

	private:
		TArray<EventHandlerId> EventHandlerIds;

		FSdkStatus& Status;
	};
}
