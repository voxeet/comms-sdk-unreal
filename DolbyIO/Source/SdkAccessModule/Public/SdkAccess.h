#pragma once

#include "Common.h"
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
	public:
		FSdkAccess(class ISdkEventsObserver&);
		~FSdkAccess();

		void Connect(const FToken&, const FConferenceName&, const FUserName&);
		void Disconnect();

		void MuteInput(const bool bIsMuted);
		void MuteOutput(const bool bIsMuted);

		void SetInputDevice(const int Index);
		void SetOutputDevice(const int Index);

		FDeviceNames GetInputDeviceNames() const;
		FDeviceNames GetOutputDeviceNames() const;

		void UpdateViewPoint(const FVector&, const FRotator&);

		void RefreshToken(const FToken&);
		FText GetStatus() const;

		dolbyio::comms::sdk* GetRawSdk();

	private:
		void Connect(const FConferenceName&, const FUserName&);
		void ConnectToDemoConference(const FUserName&);

		FSdkStatus Status;
		ISdkEventsObserver& Observer;
		FExceptionHandler ExceptionHandler;
		TUniquePtr<class FDeviceManagement> Devices;
		TUniquePtr<dolbyio::comms::sdk> Sdk;
		TUniquePtr<dolbyio::comms::refresh_token> RefreshTokenCb;
		FString LocalParticipantID;
		TArray<FString> DemoParticipantIDs;
	};
}
