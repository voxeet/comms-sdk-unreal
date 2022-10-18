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
	class SDKACCESSMODULE_API FSdkAccess final : public IModuleInterface
	{
		using FToken = FString;
		using FConferenceName = FString;
		using FUserName = FString;

	public:
		FSdkAccess();
		~FSdkAccess();

		void StartupModule() override;
		void ShutdownModule() override;

		void SetObserver(class ISdkStatusObserver*);

		void Initialize(const FToken&);
		void Connect(const FConferenceName&, const FUserName&);
		void Disconnect();

		void MuteInput(const bool bIsMuted);
		void MuteOutput(const bool bIsMuted);

		void SetInputDevice(const int Index);
		void SetOutputDevice(const int Index);

		void UpdateViewPoint(const FVector&, const FRotator&);

		void RefreshToken(const FToken&);

		dolbyio::comms::sdk* GetRawSdk();

	private:
		void LoadDll(const FString&);

		TArray<void*> DllHandles;
		FSdkStatus Status;
		TUniquePtr<dolbyio::comms::sdk> Sdk;
		TUniquePtr<dolbyio::comms::refresh_token> RefreshTokenCb;
		TUniquePtr<class FEvents> Events;
		TUniquePtr<class FDeviceManagement> Devices;
		FString LocalParticipantID;
		TSet<FString> ParticipantIDs;
		bool bIsDemo;
	};
}
