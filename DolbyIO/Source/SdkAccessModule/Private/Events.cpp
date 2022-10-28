// Copyright 2022 Dolby Laboratories

#include "Events.h"

#include "Common.h"

namespace Dolby
{
	FEvents::FEvents(FSdkStatus& Status) : Status(Status) {}

	FEvents::~FEvents()
	{
		for (auto& Id : EventHandlerIds)
		{
			Id->disconnect().on_error(DLB_HANDLE_ASYNC_EXCEPTION);
		}
	}

	void FEvents::AddEvent(EventHandlerId HandlerId) {}
}
