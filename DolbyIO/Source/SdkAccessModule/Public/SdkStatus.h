#pragma once

#include "Common.h"

namespace Dolby
{
	class ISdkEventsObserver;

	class FSdkStatus final
	{
	public:
		FSdkStatus(ISdkEventsObserver&);

		bool IsDisconnected() const;
		bool IsConnecting() const;
		bool IsConnected() const;

		void OnDisconnected();
		void OnConnecting();
		void OnConnected();

		void SetMsg(const FMessage&);

	private:
		enum class EConnectionStatus
		{
			Disconnected,
			Connecting,
			Connected
		};

		void SetConnection(EConnectionStatus);
		void OnStatusChanged();
		FMessage ToString() const;

		EConnectionStatus ConnectionStatus = EConnectionStatus::Disconnected;
		FMessage Msg;

		ISdkEventsObserver& Observer;
	};
}
