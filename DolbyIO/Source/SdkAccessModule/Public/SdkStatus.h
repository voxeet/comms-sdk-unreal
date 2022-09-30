#pragma once

namespace Dolby
{
	class ISdkApi;
	using FMessage = FString;

	class FSdkStatus final
	{
	public:
		FSdkStatus(ISdkApi&);

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

		ISdkApi& Delegate;
	};
}
