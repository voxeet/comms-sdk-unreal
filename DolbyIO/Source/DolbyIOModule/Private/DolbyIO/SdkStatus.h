// Copyright 2022 Dolby Laboratories

#pragma once

#include "Containers/UnrealString.h"

namespace dolbyio
{
	namespace comms
	{
		enum class conference_status;
	}
}

namespace DolbyIO
{
	class ISdkEventObserver;
	using EConferenceStatus = dolbyio::comms::conference_status;

	class FSdkStatus final
	{
	public:
		FSdkStatus(ISdkEventObserver&);

		bool IsConnected() const;

		void SetStatus(EConferenceStatus);
		void Log(const FString& = "");

	private:
		EConferenceStatus Status;
		ISdkEventObserver& Observer;
	};
}
