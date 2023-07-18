// Copyright 2023 Dolby Laboratories

#pragma once

#include "Async/Async.h"

namespace DolbyIO
{
	template <class TDelegate, class... TArgs> void BroadcastEvent(TDelegate& Event, TArgs&&... Args)
	{
		AsyncTask(ENamedThreads::GameThread, [=] { Event.Broadcast(Args...); });
	}
}
