#pragma once

namespace Dolby
{
	class ISdkStatusObserver;

	class FSdkStatus final
	{
	public:
		using FMessage = FString;
		using FDeviceName = FText;
		using FDeviceNames = TArray<FDeviceName>;

		FSdkStatus(ISdkStatusObserver&);

		bool IsDisconnected() const;
		bool IsConnecting() const;
		bool IsConnected() const;

		void OnDisconnected();
		void OnConnecting();
		void OnConnected();

		void OnNewListOfInputDevices(const FDeviceNames&);
		void OnNewListOfOutputDevices(const FDeviceNames&);

		void OnInputDeviceAdded(const FDeviceName&);
		void OnOutputDeviceAdded(const FDeviceName&);

		void OnInputDeviceRemoved(const FDeviceName&);
		void OnOutputDeviceRemoved(const FDeviceName&);

		void OnInputDeviceChanged(const FDeviceName&);
		void OnOutputDeviceChanged(const FDeviceName&);

		void OnRefreshTokenRequested();

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

		ISdkStatusObserver& Observer;
	};
}
