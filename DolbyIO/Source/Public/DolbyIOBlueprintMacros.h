// Copyright 2023 Dolby Laboratories

#pragma once

#include "DolbyIO.h"

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

#define DLB_DEFINE_CONSTRUCTOR(ClassName)              \
	{                                                  \
		ClassName* Self = NewObject<ClassName>();      \
		Self->WorldContextObject = WorldContextObject; \
		return Self;                                   \
	}

namespace DolbyIO
{
	inline UDolbyIOSubsystem* GetSubsystem(const UObject* WorldContextObject)
	{
		UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
		return GameInstance ? GameInstance->GetSubsystem<UDolbyIOSubsystem>() : nullptr;
	}
}

#define DLB_GET_SUBSYSTEM                                                            \
	UDolbyIOSubsystem* DolbyIOSubsystem = DolbyIO::GetSubsystem(WorldContextObject); \
	if (!DolbyIOSubsystem)                                                           \
	{                                                                                \
		return;                                                                      \
	}

#define DLB_UNBIND_EVENT(MethodName, Event) \
	DolbyIOSubsystem->Event.RemoveDynamic(this, &UDolbyIO##MethodName::Event##Impl);

#define DLB_UNBIND_EVENTS_BASE(MethodName, Event)                                                    \
	DLB_GET_SUBSYSTEM;                                                                               \
	DolbyIOSubsystem->On##MethodName##Error.RemoveDynamic(this, &UDolbyIO##MethodName::OnErrorImpl); \
	DLB_UNBIND_EVENT(MethodName, Event);

#define DLB_BIND_EVENT(MethodName, Event) DolbyIOSubsystem->Event.AddDynamic(this, &UDolbyIO##MethodName::Event##Impl);

#define DLB_ACTIVATE_BASE(MethodName, Event)                                                      \
	DLB_GET_SUBSYSTEM;                                                                            \
	DolbyIOSubsystem->On##MethodName##Error.AddDynamic(this, &UDolbyIO##MethodName::OnErrorImpl); \
	DLB_BIND_EVENT(MethodName, Event);

#define DLB_DEFINE_ACTIVATE_METHOD_OneEvent(MethodName, Event1, ...) \
	void Activate() override                                         \
	{                                                                \
		DLB_ACTIVATE_BASE(MethodName, Event1);                       \
		DolbyIOSubsystem->MethodName(__VA_ARGS__);                   \
	}                                                                \
	void UnbindEvents()                                              \
	{                                                                \
		DLB_UNBIND_EVENTS_BASE(MethodName, Event1);                  \
	}

#define DLB_DEFINE_ACTIVATE_METHOD_TwoEvents(MethodName, Event1, Event2, ...) \
	void Activate() override                                                  \
	{                                                                         \
		DLB_ACTIVATE_BASE(MethodName, Event1);                                \
		DLB_BIND_EVENT(MethodName, Event2);                                   \
		DolbyIOSubsystem->MethodName(__VA_ARGS__);                            \
	}                                                                         \
	void UnbindEvents()                                                       \
	{                                                                         \
		DLB_UNBIND_EVENTS_BASE(MethodName, Event1);                           \
		DLB_UNBIND_EVENT(MethodName, Event2);                                 \
	}

#define DLB_DEFINE_IMPL_METHOD(Event, ...) \
	{                                      \
		Event.Broadcast(__VA_ARGS__);      \
		UnbindEvents();                    \
	}

#define DLB_DEFINE_ERROR_METHOD(...) DLB_DEFINE_IMPL_METHOD(OnError, __VA_ARGS__);

#define DLB_EXECUTE_SUBSYSTEM_METHOD(MethodName, ...) \
	DLB_GET_SUBSYSTEM;                                \
	DolbyIOSubsystem->MethodName(__VA_ARGS__);

#define DLB_EXECUTE_RETURNING_SUBSYSTEM_METHOD(MethodName, ...)                      \
	UDolbyIOSubsystem* DolbyIOSubsystem = DolbyIO::GetSubsystem(WorldContextObject); \
	if (!DolbyIOSubsystem)                                                           \
	{                                                                                \
		return {};                                                                   \
	}                                                                                \
	return DolbyIOSubsystem->MethodName(__VA_ARGS__);
