#include "SdkStatus.h"
#include "SdkApi.h"

DEFINE_LOG_CATEGORY_STATIC(LogDolby, Log, All);
#define DLB_UE_LOG(Format, ...) UE_LOG(LogDolby, Log, TEXT(Format), ##__VA_ARGS__)

namespace Dolby
{
	FSdkStatus::FSdkStatus(ISdkApi& Delegate) : Delegate(Delegate) {}

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
		Delegate.OnStatusChanged(Status);
		Msg.Reset();
	}

	FMessage FSdkStatus::ToString() const
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
