// Copyright 2022 Dolby Laboratories

#pragma once

#include "DolbyIO/Typedefs.h"

namespace dolbyio
{
	namespace comms
	{
		class refresh_token;
		class sdk;
	}
}

namespace DolbyIO
{
	struct FErrorHandler;
	class FSdkStatus;
	class ISdkEventObserver;

	class FSdkAccess final
	{
		using FToken = FString;
		using FConferenceName = FString;
		using FUserName = FString;

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
		FErrorHandler MakeHandler(int Id);

		ISdkEventObserver& Observer;
		TUniquePtr<FSdkStatus> Status;
		TUniquePtr<dolbyio::comms::sdk> Sdk;
		TUniquePtr<dolbyio::comms::refresh_token> RefreshTokenCb;
		TUniquePtr<class FDeviceManagement> Devices;
		FParticipant LocalParticipantID;
		FParticipants ParticipantIDs;
		bool bIsDemo;
	};
}
