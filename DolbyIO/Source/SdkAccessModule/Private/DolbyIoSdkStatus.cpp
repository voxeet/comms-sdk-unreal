// Copyright 2022 Dolby Laboratories

#include "DolbyIoSdkStatus.h"

#include "DolbyIoLogging.h"
#include "DolbyIoSdkEventObserver.h"

#include <dolbyio/comms/conference.h>

namespace Dolby
{
	FSdkStatus::FSdkStatus(ISdkEventObserver& Observer) : Observer(Observer) {}

	bool FSdkStatus::IsConnected() const
	{
		return Status == EConferenceStatus::joined;
	}

	namespace
	{
		FString ToString(EConferenceStatus Status)
		{
			switch (Status)
			{
				case EConferenceStatus::creating:
					return "creating";
				case EConferenceStatus::created:
					return "created";
				case EConferenceStatus::joining:
					return "joining";
				case EConferenceStatus::joined:
					return "joined";
				case EConferenceStatus::leaving:
					return "leaving";
				case EConferenceStatus::left:
					return "left";
				case EConferenceStatus::destroyed:
					return "destroyed";
				case EConferenceStatus::error:
					return "error";
				default:
					return "unknown";
			};
		}
	}

	void FSdkStatus::SetStatus(EConferenceStatus ConferenceStatus)
	{
		Status = ConferenceStatus;

		switch (Status)
		{
			case EConferenceStatus::joined:
				Observer.OnConnectedEvent();
				break;
			case EConferenceStatus::left:
			case EConferenceStatus::destroyed:
			case EConferenceStatus::error:
				Observer.OnDisconnectedEvent();
				break;
		}

		Log();
	}

	void FSdkStatus::Log(const FString& Msg)
	{
		FString LogMsg = ToString(Status);
		if (!Msg.IsEmpty())
		{
			LogMsg += " - " + Msg;
		}
		DLB_UE_LOG("Conference status: %s", *LogMsg);
	}
}
