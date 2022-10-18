// Copyright 2022 Dolby Laboratories

#pragma once

#include "SdkStatusObserver.h"

namespace Dolby
{
	class FSdkStatus final
	{
	public:
		void SetObserver(ISdkStatusObserver*);

		bool IsDisconnected() const;
		bool IsConnecting() const;
		bool IsConnected() const;

		void OnDisconnected();
		void OnConnecting();
		void OnConnected();
		void OnDisconnecting();

		void OnNewListOfInputDevices(const FDeviceNames&);
		void OnNewListOfOutputDevices(const FDeviceNames&);

		void OnInputDeviceAdded(const FDeviceName&);
		void OnOutputDeviceAdded(const FDeviceName&);

		void OnInputDeviceRemoved(const FDeviceName&);
		void OnOutputDeviceRemoved(const FDeviceName&);

		void OnInputDeviceChanged(const FDeviceName&);
		void OnOutputDeviceChanged(const FDeviceName&);

		void OnLocalParticipantChanged(const FParticipant&);
		void OnNewListOfRemoteParticipants(const FParticipants&);
		void OnNewListOfActiveSpeakers(const FParticipants&);
		void OnNewAudioLevels(const FAudioLevels&);

		void OnRefreshTokenRequested();

		void SetMsg(const FMessage&);

	private:
		enum class EConnectionStatus
		{
			Disconnected,
			Connecting,
			Connected,
			Disconnecting
		};

		void SetConnection(EConnectionStatus);
		void OnStatusChanged();
		FMessage ToString() const;

		EConnectionStatus ConnectionStatus = EConnectionStatus::Disconnected;
		FMessage Msg;

		ISdkStatusObserver* Observer;
	};
}
