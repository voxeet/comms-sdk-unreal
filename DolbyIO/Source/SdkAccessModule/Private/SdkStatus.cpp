// Copyright 2022 Dolby Laboratories

#include "SdkStatus.h"

#include "Common.h"
#include "SdkEventsObserver.h"

#include <dolbyio/comms/conference.h>

DEFINE_LOG_CATEGORY(LogDolby);

namespace Dolby
{
	inline FMessage ToString(EConferenceStatus Status)
	{
		switch (Status)
		{
			case dolbyio::comms::conference_status::creating:
				return "Disconnected (creating)";
			case dolbyio::comms::conference_status::created:
				return "created";
			case dolbyio::comms::conference_status::joining:
				return "joining";
			case dolbyio::comms::conference_status::joined:
				return "Connected (joined)";
			case dolbyio::comms::conference_status::leaving:
				return "leaving";
			case dolbyio::comms::conference_status::left:
				return "Disconnected (left)";
			case dolbyio::comms::conference_status::destroyed:
				return "Disconnected (destroyed)";
			case dolbyio::comms::conference_status::error:
				return "Disconnected (error)";
			default:
				return "unknown";
		};
	}

	void FSdkStatus::SetObserver(ISdkEventsObserver* AnObserver)
	{
		Observer = AnObserver;
		UpdateStatus = &FSdkStatus::NotifyObserver;
	}

	bool FSdkStatus::IsConnected() const
	{
		return ConferenceStatus == EConferenceStatus::joined;
	}

	void FSdkStatus::SetMsg(const FMessage& Msg)
	{
		(this->*UpdateStatus)(ToString(ConferenceStatus) + " - " + Msg);
	}

	void FSdkStatus::SetStatus(dolbyio::comms::conference_status Status)
	{
		ConferenceStatus = Status;
		(this->*UpdateStatus)(ToString(ConferenceStatus));
	}

	void FSdkStatus::NotifyObserver(const FMessage& Msg)
	{
		LogMsg(Msg);
		Observer->OnStatusChanged(Msg);
	}

	void FSdkStatus::LogMsg(const FMessage& Msg)
	{
		DLB_UE_LOG("Status: %s", *Msg);
	}

}
