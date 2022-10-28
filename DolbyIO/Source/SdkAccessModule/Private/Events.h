// Copyright 2022 Dolby Laboratories

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_WINDOWS
#pragma warning(push)
#pragma warning(disable : 4251)
#pragma warning(disable : 4275)
#endif
#include <dolbyio/comms/event_handling.h>
#if PLATFORM_WINDOWS
#pragma warning(pop)
#endif

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
