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
	PrimaryActorTick.TickInterval = 0.3;
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

void ADolbyIoConference::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Position;
	FRotator Rotation;
	GetWorld()->GetFirstPlayerController()->GetActorEyesViewPoint(Position, Rotation);
	CppSdk->UpdateViewPoint(Position, Rotation);
}

#define ON_GAME_THREAD(Func) AsyncTask(ENamedThreads::GameThread, [this] { Func(); });

void ADolbyIoConference::OnStatusChanged(const FMessage& Msg)
{
	Status = Msg;
	ON_GAME_THREAD(OnStatusChanged);
}

void ADolbyIoConference::OnNewListOfInputDevices(const FDeviceNames& Names)
{
	InputDevices = Names;
	ON_GAME_THREAD(OnNewListOfInputDevices);
}

void ADolbyIoConference::OnNewListOfOutputDevices(const FDeviceNames& Names)
{
	OutputDevices = Names;
	ON_GAME_THREAD(OnNewListOfOutputDevices);
}

void ADolbyIoConference::OnInputDeviceChanged(const FDeviceName& Name)
{
	CurrentInputDevice = Name;
	ON_GAME_THREAD(OnInputDeviceChanged);
}

void ADolbyIoConference::OnOutputDeviceChanged(const FDeviceName& Name)
{
	CurrentOutputDevice = Name;
	ON_GAME_THREAD(OnOutputDeviceChanged);
}

void ADolbyIoConference::OnRefreshTokenRequested()
{
	ON_GAME_THREAD(OnRefreshTokenNeeded);
}
