// Copyright 2023 Dolby Laboratories

#include "DolbyIO.h"

#include "Utils/DolbyIOBroadcastEvent.h"
#include "Utils/DolbyIOConversions.h"
#include "Utils/DolbyIOErrorHandler.h"
#include "Utils/DolbyIOLogging.h"
#include "Video/DolbyIOVideoFrameHandler.h"
#include "Video/DolbyIOVideoProcessingFrameHandler.h"

using namespace dolbyio::comms;
using namespace DolbyIO;

void UDolbyIOSubsystem::EnableVideo(const FDolbyIOVideoDevice& VideoDevice, bool bBlurBackground)
{
	if (!Sdk)
	{
		DLB_UE_WARN("Cannot enable video - not initialized");
		return;
	}

	DLB_UE_LOG("Enabling video");

	std::shared_ptr<video_frame_handler> VideoFrameHandler = LocalCameraFrameHandler;
	if (bBlurBackground)
	{
#if PLATFORM_WINDOWS | PLATFORM_MAC
		DLB_UE_LOG("Blurring background");
		VideoFrameHandler =
		    std::make_shared<FVideoProcessingFrameHandler>(VideoProcessor, LocalCameraFrameHandler->sink());
#else
		DLB_UE_WARN("Cannot blur background on this platform");
#endif
	}

	Sdk->video()
	    .local()
	    .start(ToSdkVideoDevice(VideoDevice), VideoFrameHandler)
	    .then(
	        [this]
	        {
		        bIsVideoEnabled = true;
		        BroadcastEvent(OnVideoEnabled, LocalCameraTrackID);
	        })
	    .on_error(DLB_ERROR_HANDLER);
}

void UDolbyIOSubsystem::DisableVideo()
{
	if (!Sdk)
	{
		return;
	}

	DLB_UE_LOG("Disabling video");
	Sdk->video()
	    .local()
	    .stop()
	    .then(
	        [this]
	        {
		        bIsVideoEnabled = false;
		        BroadcastEvent(OnVideoDisabled, LocalCameraTrackID);
	        })
	    .on_error(DLB_ERROR_HANDLER);
}
