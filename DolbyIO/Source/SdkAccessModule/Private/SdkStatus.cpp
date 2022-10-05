#include "SdkStatus.h"

DEFINE_LOG_CATEGORY_STATIC(LogDolby, Log, All);
#define DLB_UE_LOG(Format, ...) UE_LOG(LogDolby, Log, TEXT(Format), ##__VA_ARGS__)
#define DLB_UE_LOG_DEVICE(Type, Event) DLB_UE_LOG(Type " device " Event ": %s", *Name.ToString())

namespace Dolby
{
	FSdkStatus::FSdkStatus(ISdkStatusObserver& Observer) : Observer(Observer) {}

	bool FSdkStatus::IsDisconnected() const
	{
		return ConnectionStatus == EConnectionStatus::Disconnected;
	}
	bool FSdkStatus::IsConnecting() const
	{
		return ConnectionStatus == EConnectionStatus::Connecting;
	}
	bool FSdkStatus::IsConnected() const
	{
		return ConnectionStatus == EConnectionStatus::Connected;
	}

	void FSdkStatus::OnDisconnected()
	{
		SetConnection(EConnectionStatus::Disconnected);
	}
	void FSdkStatus::OnConnecting()
	{
		SetConnection(EConnectionStatus::Connecting);
	}
	void FSdkStatus::OnConnected()
	{
		SetConnection(EConnectionStatus::Connected);
	}

	void FSdkStatus::OnNewListOfInputDevices(const FDeviceNames& Names)
	{
		Observer.OnNewListOfInputDevices(Names);
	}
	void FSdkStatus::OnNewListOfOutputDevices(const FDeviceNames& Names)
	{
		Observer.OnNewListOfOutputDevices(Names);
	}
	void FSdkStatus::OnInputDeviceAdded(const FDeviceName& Name)
	{
		DLB_UE_LOG_DEVICE("Input", "added");
	}
	void FSdkStatus::OnOutputDeviceAdded(const FDeviceName& Name)
	{
		DLB_UE_LOG_DEVICE("Output", "added");
	}
	void FSdkStatus::OnInputDeviceRemoved(const FDeviceName& Name)
	{
		DLB_UE_LOG_DEVICE("Input", "removed");
	}
	void FSdkStatus::OnOutputDeviceRemoved(const FDeviceName& Name)
	{
		DLB_UE_LOG_DEVICE("Output", "removed");
	}
	void FSdkStatus::OnInputDeviceChanged(const FDeviceName& Name)
	{
		DLB_UE_LOG_DEVICE("Input", "changed");
		Observer.OnInputDeviceChanged(Name);
	}
	void FSdkStatus::OnOutputDeviceChanged(const FDeviceName& Name)
	{
		DLB_UE_LOG_DEVICE("Output", "changed");
		Observer.OnOutputDeviceChanged(Name);
	}

	void FSdkStatus::OnRefreshTokenRequested()
	{
		DLB_UE_LOG("Refresh token requested");
		Observer.OnRefreshTokenRequested();
	}

	void FSdkStatus::SetMsg(const FMessage& M)
	{
		Msg = M;
		IsConnecting() ? OnDisconnected() : OnStatusChanged();
	}

	void FSdkStatus::SetConnection(EConnectionStatus S)
	{
		ConnectionStatus = S;
		OnStatusChanged();
	}

	void FSdkStatus::OnStatusChanged()
	{
		const FMessage Status = ToString();
		DLB_UE_LOG("%s", *Status);
		Observer.OnStatusChanged(Status);
		Msg.Reset();
	}

	FSdkStatus::FMessage FSdkStatus::ToString() const
	{
		FMessage Ret;
		switch (ConnectionStatus)
		{
			case EConnectionStatus::Disconnected:
				Ret = "Disconnected";
				break;
			case EConnectionStatus::Connecting:
				Ret = "Connecting";
				break;
			case EConnectionStatus::Connected:
				Ret = "Connected";
				break;
		}
		if (!Msg.IsEmpty())
		{
			Ret += " - " + Msg;
		}
		return Ret;
	}
}
