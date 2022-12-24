// Copyright 2022 Dolby Laboratories

#pragma once

#include "DolbyIO/Typedefs.h"

namespace dolbyio::comms
{
	enum class conference_status;
	class refresh_token;
	class sdk;
}

namespace DolbyIO
{
	struct FErrorHandler;
	class ISdkEventObserver;

	class FSdkAccess final
	{
		using FToken = FString;
		using FConferenceName = FString;
		using FUserName = FString;
		using EConferenceStatus = dolbyio::comms::conference_status;

	public:
		FSdkAccess(ISdkEventObserver&);
		~FSdkAccess();

		void SetToken(const FToken&);

		void Connect(const FConferenceName&, const FUserName&);
		void Disconnect();

		void UpdateViewPoint(const FVector&, const FRotator&);

		void MuteInput();
		void UnmuteInput();
		void MuteOutput();
		void UnmuteOutput();

		void GetAudioLevels();

		void SetInputDevice(int Index);
		void SetOutputDevice(int Index);

	private:
		void Initialize(const FToken&);

		bool IsConnected() const;

		void UpdateStatus(EConferenceStatus);
		void Log(const FString& = "") const;

		FErrorHandler MakeHandler(int Id);

		ISdkEventObserver& Observer;
		TUniquePtr<dolbyio::comms::sdk> Sdk;
		TUniquePtr<dolbyio::comms::refresh_token> RefreshTokenCb;
		TUniquePtr<class FDeviceManagement> Devices;
		EConferenceStatus ConferenceStatus;
		FParticipant LocalParticipantID;
		FParticipants ParticipantIDs;
		bool bIsDemo;
	};
}
