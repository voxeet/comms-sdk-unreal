#include "DolbyIoConference.h"
#include "Modules/ModuleManager.h"
#include "SdkAccess.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, DolbyIoConferenceModule)

ADolbyIoConference::ADolbyIoConference()
    // these defaults can be overriden in Unreal Editor
    : ConferenceName("unreal"), UserName("unreal"), Status("Disconnected"), CppSdk(MakeShared<Dolby::FSdkAccess>(*this))
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.03;
}

void ADolbyIoConference::Connect()
{
	CppSdk->Connect(Token, ConferenceName, UserName);
}
void ADolbyIoConference::Disconnect()
{
	CppSdk->Disconnect();
}
void ADolbyIoConference::MuteInput()
{
	CppSdk->MuteInput(bIsInputMuted);
}
void ADolbyIoConference::MuteOutput()
{
	CppSdk->MuteOutput(bIsOutputMuted);
}
void ADolbyIoConference::SetInputDevice(const int Index)
{
	CppSdk->SetInputDevice(Index);
}
void ADolbyIoConference::SetOutputDevice(const int Index)
{
	CppSdk->SetOutputDevice(Index);
}
void ADolbyIoConference::RefreshToken()
{
	CppSdk->RefreshToken(Token);
}

dolbyio::comms::sdk* ADolbyIoConference::GetRawSdk()
{
	return CppSdk->GetRawSdk();
}

void ADolbyIoConference::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	OnSpatialUpdateNeeded();
	CppSdk->UpdateViewPoint(Position, Rotation);
}

inline void ADolbyIoConference::RunOnGameThread(void (ADolbyIoConference::*Function)())
{
	AsyncTask(ENamedThreads::GameThread, [this, Function] { (this->*Function)(); });
}

void ADolbyIoConference::OnStatusChanged(const Dolby::FMessage& Msg)
{
	Status = Msg;
	RunOnGameThread(&ADolbyIoConference::OnStatusChanged);
}

void ADolbyIoConference::OnListOfInputDevicesChanged()
{
	RunOnGameThread(&ADolbyIoConference::OnNewListOfInputDevices);
}

void ADolbyIoConference::OnListOfOutputDevicesChanged()
{
	RunOnGameThread(&ADolbyIoConference::OnNewListOfOutputDevices);
}

void ADolbyIoConference::OnInputDeviceChanged(int Index)
{
	CurrentInputDeviceIndex = Index;
	RunOnGameThread(&ADolbyIoConference::OnInputDeviceChanged);
}

void ADolbyIoConference::OnOutputDeviceChanged(int Index)
{
	CurrentOutputDeviceIndex = Index;
	RunOnGameThread(&ADolbyIoConference::OnOutputDeviceChanged);
}

void ADolbyIoConference::OnRefreshTokenRequested()
{
	RunOnGameThread(&ADolbyIoConference::OnRefreshTokenNeeded);
}

void ADolbyIoConference::OnSpatialUpdateNeeded_Implementation()
{
	if (const auto world = GetWorld())
	{
		if (const auto player = world->GetFirstPlayerController())
		{
			player->GetActorEyesViewPoint(Position, Rotation);
		}
	}
}
DEFINE_LOG_CATEGORY(LogDolby);

TArray<FText> ADolbyIoConference::GetInputDevices() const
{
	UE_LOG(LogDolby, Log, TEXT("GetInputDevices()"));
	return CppSdk->GetInputDeviceNames();
}

TArray<FText> ADolbyIoConference::GetOutputDevices() const
{
	UE_LOG(LogDolby, Log, TEXT("GetOutputDevices()"));
	return CppSdk->GetOutputDeviceNames();
}
