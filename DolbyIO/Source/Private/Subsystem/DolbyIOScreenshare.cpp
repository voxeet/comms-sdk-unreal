// Copyright 2023 Dolby Laboratories

#include "DolbyIO.h"

#include "Utils/DolbyIOBroadcastEvent.h"
#include "Utils/DolbyIOConversions.h"
#include "Utils/DolbyIOErrorHandler.h"
#include "Utils/DolbyIOLogging.h"
#include "Video/DolbyIOVideoFrameHandler.h"

using namespace dolbyio::comms;
using namespace DolbyIO;

void UDolbyIOSubsystem::GetScreenshareSources()
{
	if (!Sdk)
	{
		DLB_WARNING(OnGetScreenshareSourcesError, "Cannot get screenshare sources - not initialized");
		return;
	}

	DLB_UE_LOG("Getting screenshare sources");
	Sdk->device_management()
	    .get_screen_share_sources()
	    .then(
	        [this](const std::vector<screen_share_source>& ScreenShareSource)
	        {
		        TArray<FDolbyIOScreenshareSource> Sources;
		        for (const screen_share_source& Source : ScreenShareSource)
		        {
			        Sources.Add(FDolbyIOScreenshareSource{
			            Source.id, Source.type == screen_share_source::type::screen,
			            Source.title.empty() ? FText::FromString(FString{"Screen "} + FString::FromInt(Source.id + 1))
			                                 : ToFText(Source.title)});
		        }
		        BroadcastEvent(OnScreenshareSourcesReceived, Sources);
	        })
	    .on_error(DLB_ERROR_HANDLER(OnGetScreenshareSourcesError));
}

void UDolbyIOSubsystem::StartScreenshare(const FDolbyIOScreenshareSource& Source,
                                         EDolbyIOScreenshareEncoderHint EncoderHint,
                                         EDolbyIOScreenshareMaxResolution MaxResolution,
                                         EDolbyIOScreenshareDownscaleQuality DownscaleQuality)
{
	if (!IsConnectedAsActive())
	{
		DLB_WARNING(OnStartScreenshareError, "Cannot start screenshare - not connected as active user");
		return;
	}

	DLB_UE_LOG("Starting screenshare using source: ID=%d IsScreen=%d Title=%s %s %s %s", Source.ID, Source.bIsScreen,
	           *Source.Title.ToString(), *UEnum::GetValueAsString(EncoderHint), *UEnum::GetValueAsString(MaxResolution),
	           *UEnum::GetValueAsString(DownscaleQuality));
	Sdk->conference()
	    .start_screen_share(screen_share_source{ToStdString(Source.Title.ToString()), Source.ID,
	                                            Source.bIsScreen ? screen_share_source::type::screen
	                                                             : screen_share_source::type::window},
	                        LocalScreenshareFrameHandler,
	                        ToSdkContentInfo(EncoderHint, MaxResolution, DownscaleQuality))
	    .then([this] { BroadcastEvent(OnScreenshareStarted, LocalScreenshareTrackID); })
	    .on_error(DLB_ERROR_HANDLER(OnStartScreenshareError));
}

void UDolbyIOSubsystem::StopScreenshare()
{
	if (!Sdk)
	{
		return;
	}

	DLB_UE_LOG("Stopping screenshare");
	Sdk->conference()
	    .stop_screen_share()
	    .then([this] { BroadcastEvent(OnScreenshareStopped, LocalScreenshareTrackID); })
	    .on_error(DLB_ERROR_HANDLER(OnStopScreenshareError));
}

void UDolbyIOSubsystem::ChangeScreenshareParameters(EDolbyIOScreenshareEncoderHint EncoderHint,
                                                    EDolbyIOScreenshareMaxResolution MaxResolution,
                                                    EDolbyIOScreenshareDownscaleQuality DownscaleQuality)
{
	if (!IsConnectedAsActive())
	{
		return;
	}
	DLB_UE_LOG("Changing screenshare parameters to %s %s %s", *UEnum::GetValueAsString(EncoderHint),
	           *UEnum::GetValueAsString(MaxResolution), *UEnum::GetValueAsString(DownscaleQuality));
	Sdk->conference()
	    .screen_share_content_info(ToSdkContentInfo(EncoderHint, MaxResolution, DownscaleQuality))
	    .on_error(DLB_ERROR_HANDLER(OnChangeScreenshareParametersError));
}
