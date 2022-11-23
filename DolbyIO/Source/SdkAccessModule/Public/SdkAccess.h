// Copyright 2022 Dolby Laboratories

#pragma once

#include "SdkStatus.h"

#include "Modules/ModuleManager.h"

namespace dolbyio
{
	namespace comms
	{
		class refresh_token;
		class sdk;
	}
}

namespace Dolby
{
	struct FErrorHandler;

	class SDKACCESSMODULE_API FSdkAccess final : public IModuleInterface
	{
	public:
		FSdkAccess();
		~FSdkAccess();

		void StartupModule() override;
		void ShutdownModule() override;

		FErrorHandler MakeHandler(int Id);
		void SetObserver(class ISdkEventsObserver*);

		void Connect(const FToken&, const FConferenceName&, const FUserName&);
		void Disconnect();
		void ShutDown();

		void MuteInput(const bool bIsMuted);
		void MuteOutput(const bool bIsMuted);

		void SetInputDevice(const int Index);
		void SetOutputDevice(const int Index);

		FDeviceNames GetInputDeviceNames() const;
		FDeviceNames GetOutputDeviceNames() const;
		Index GetNumberOfInputDevices() const;
		Index GetNumberOfOutputDevices() const;

		void UpdateViewPoint(const FVector&, const FRotator&);

		void GetAudioLevels();

		void RefreshToken(const FToken&);

		dolbyio::comms::sdk* GetRawSdk();

	private:
		void LoadDll(const FString&);
		void Initialize(const FToken&);
		void WaitForDisconnect();

		TArray<void*> DllHandles;
		ISdkEventsObserver* Observer = nullptr;
		FSdkStatus Status;
		TUniquePtr<dolbyio::comms::sdk> Sdk;
		TUniquePtr<dolbyio::comms::refresh_token> RefreshTokenCb;
		TUniquePtr<class FDeviceManagement> Devices;
		FString LocalParticipantID;
		TSet<FString> ParticipantIDs;
		bool bIsDemo;
	};
}
