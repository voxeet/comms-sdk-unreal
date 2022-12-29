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
	class ISdkEventObserver;

	class FSdkAccess final
	{
		using FToken = FString;

	public:
		using EConferenceStatus = dolbyio::comms::conference_status;

		FSdkAccess(ISdkEventObserver&);
		~FSdkAccess();

		void SetToken(const FToken&);

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
		void Initialize(const FToken&);
		bool IsConnected() const;
		bool CanConnect() const;
		void UpdateStatus(EConferenceStatus);
		class FErrorHandler MakeHandler(int Line);

		ISdkEventObserver& Observer;
		TUniquePtr<dolbyio::comms::sdk> Sdk;
		TUniquePtr<dolbyio::comms::refresh_token> RefreshTokenCb;
		EConferenceStatus ConferenceStatus;
		FParticipantID LocalParticipantID;
		FParticipantIDs RemoteParticipantIDs;
		bool bIsDemo;
	};
}
