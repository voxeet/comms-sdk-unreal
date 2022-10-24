#pragma once

#include "Common.h"

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

		virtual void OnRefreshTokenRequested() = 0;
	};
}
