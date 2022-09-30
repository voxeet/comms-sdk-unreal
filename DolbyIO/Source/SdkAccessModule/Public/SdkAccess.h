#pragma once

#include "SdkStatus.h"
#include "ExceptionHandler.h"

namespace dolbyio
{
	namespace comms
	{
		class refresh_token;
		class sdk;
	} // namespace comms
} // namespace dolbyio

namespace Dolby
{
	class SDKACCESSMODULE_API FSdkAccess final
	{
		using FToken = FString;
		using FConferenceName = FString;
		using FUserName = FString;

	public:
		FSdkAccess(class ISdkApi&);
		~FSdkAccess();

		void Connect(const FToken&, const FConferenceName&, const FUserName&);
		void Disconnect();

		void MuteInput(const bool bIsMuted);
		void MuteOutput(const bool bIsMuted);

		void SetInputDevice(const int Index);
		void SetOutputDevice(const int Index);

		void UpdateViewPoint(const FVector&, const FRotator&);

		void RefreshToken(const FToken&);

		dolbyio::comms::sdk* GetRawSdk();

	private:
		void Connect(const FConferenceName&, const FUserName&);
		void ConnectToDemoConference(const FUserName&);

		FSdkStatus Status;
		ISdkApi& Delegate;
		FExceptionHandler ExceptionHandler;
		TUniquePtr<class FDeviceManagement> Devices;
		TUniquePtr<dolbyio::comms::sdk> Sdk;
		TUniquePtr<dolbyio::comms::refresh_token> RefreshTokenCb;
		FString LocalParticipantID;
		TArray<FString> DemoParticipantIDs;
	};
}
