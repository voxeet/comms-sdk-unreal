// Copyright 2023 Dolby Laboratories

#include "Utils/DolbyIOErrorHandler.h"

#include "DolbyIO.h"
#include "Utils/DolbyIOBroadcastEvent.h"
#include "Utils/DolbyIOConversions.h"
#include "Utils/DolbyIOLogging.h"

#include "Misc/Paths.h"

namespace DolbyIO
{
	using namespace dolbyio::comms;

	FErrorHandler::FErrorHandler(const FString& File, int Line, UDolbyIOSubsystem& DolbyIOSubsystem)
	    : File(FPaths::GetCleanFilename(File)), Line(Line), DolbyIOSubsystem(DolbyIOSubsystem)
	{
	}

	FErrorHandler::FErrorHandler(const FString& File, int Line, UDolbyIOSubsystem& DolbyIOSubsystem,
	                             const FDolbyIOOnErrorDelegate& OnError)
	    : File(FPaths::GetCleanFilename(File)), Line(Line), DolbyIOSubsystem(DolbyIOSubsystem), OnError(&OnError)
	{
	}

	void FErrorHandler::operator()(std::exception_ptr&& ExcPtr) const
	{
		HandleError([ExcP = MoveTemp(ExcPtr)] { std::rethrow_exception(ExcP); });
	}

	void FErrorHandler::HandleError() const
	{
		HandleError([] { throw; });
	}

#define DLB_CATCH(Exception)                                    \
	catch (const dolbyio::comms::Exception& Ex)                 \
	{                                                           \
		LogException("dolbyio::comms::" #Exception, Ex.what()); \
	}
	void FErrorHandler::HandleError(TFunction<void()> Callee) const
	try
	{
		Callee();
	}
	DLB_CATCH(async_operation_canceled)               // : exception
	DLB_CATCH(certificate_exception)                  // : exception
	DLB_CATCH(conference_state_exception)             // : conference_exception
	DLB_CATCH(dvc_error_exception)                    // : media_engine_exception
	DLB_CATCH(create_answer_exception)                // : media_engine_exception
	DLB_CATCH(create_peer_connection_exception)       // : media_engine_exception
	DLB_CATCH(ice_candidate_exception)                // : media_engine_exception
	DLB_CATCH(media_stream_exception)                 // : media_engine_exception
	DLB_CATCH(peer_connection_disconnected_exception) // : media_engine_exception
	DLB_CATCH(peer_connection_failed_exception)       // : media_engine_exception
	DLB_CATCH(sdp_exception)                          // : media_engine_exception
	DLB_CATCH(media_engine_exception)                 // : conference_exception
	DLB_CATCH(conference_exception)                   // : exception
	DLB_CATCH(http_exception)                         // : io_exception
	DLB_CATCH(invalid_token_exception)                // : restapi_exception
	DLB_CATCH(restapi_exception)                      // : io_exception
	DLB_CATCH(security_check_exception)               // : io_exception
	DLB_CATCH(signaling_channel_exception)            // : io_exception
	DLB_CATCH(io_exception)                           // : exception
	DLB_CATCH(json_exception)                         // : exception
	DLB_CATCH(jwt_exception)                          // : exception
	DLB_CATCH(dvc_exception)                          // : media_exception
	DLB_CATCH(media_exception)                        // : exception
	DLB_CATCH(session_exception)                      // : exception
	DLB_CATCH(spatial_placement_exception)            // : exception
	DLB_CATCH(exception)
	catch (const std::exception& Ex)
	{
		LogException("std::exception", Ex.what());
	}
	catch (...)
	{
		LogException("unknown exception", "");
	}

	void FErrorHandler::LogException(const FString& Type, const FString& What) const
	{
		const FString ErrorMsg = Type + ": " + What;
		DLB_UE_LOG_BASE(Error, "Caught %s (conference status: %s, %s:%d)", *ErrorMsg,
		                *ToString(DolbyIOSubsystem.ConferenceStatus), *File, Line);
		if (OnError)
		{
			BroadcastEvent(*OnError, ErrorMsg);
		}
	}

	void FErrorHandler::Warn(const FDolbyIOOnErrorDelegate& OnError, const FString& Msg)
	{
		DLB_UE_LOG_BASE(Warning, "%s", *Msg);
		BroadcastEvent(OnError, Msg);
	}
}
