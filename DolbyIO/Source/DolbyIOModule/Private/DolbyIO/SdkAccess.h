// Copyright 2022 Dolby Laboratories

#pragma once

#include "Containers/UnrealString.h"
#include "Templates/UniquePtr.h"

namespace dolbyio::comms
{
	enum class conference_status;
	class refresh_token;
	class sdk;
}

class UDolbyIOSubsystem;

namespace DolbyIO
{

	class FSdkAccess final
	{
	public:
		FSdkAccess(UDolbyIOSubsystem&);
		~FSdkAccess();

		void SetToken(const FString& Token);

		void Connect(const FString& ConferenceName, const FString& UserName, const FString& ExternalID,
		             const FString& AvatarURL);
		void ConnectToDemoConference();
		void Disconnect();

		void UpdateViewPoint(const FVector&, const FRotator&);

		void MuteInput();
		void UnmuteInput();
		void MuteOutput();
		void UnmuteOutput();

		void GetAudioLevels();

	private:
		void Initialize(const FString& Token);
		bool IsConnected() const;
		bool CanConnect() const;
		void UpdateStatus(dolbyio::comms::conference_status);
		class FErrorHandler MakeErrorHandler(int Line);

		UDolbyIOSubsystem& DolbyIOSubsystem;

		dolbyio::comms::conference_status ConferenceStatus;
		FString LocalParticipantID;
		TArray<FString> RemoteParticipantIDs;

		TUniquePtr<dolbyio::comms::sdk> Sdk;
		TUniquePtr<dolbyio::comms::refresh_token> RefreshTokenCb;

		bool bIsAlive = true;
		bool bIsDemo;
	};
}
