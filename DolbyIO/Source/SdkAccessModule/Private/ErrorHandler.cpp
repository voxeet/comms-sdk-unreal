#include "ErrorHandler.h"

#include "dolbyio/comms/sdk_exceptions.h"

namespace Dolby
{

    FErrorHandler::FErrorHandler(FStatusUpdater UpdateStatus, FDisconnector Disconnect)
        : UpdateStatus(UpdateStatus), Disconnect(Disconnect)
    {
    }

    void FErrorHandler::operator()(std::exception_ptr&& ExcPtr)
    {
        NotifyIfThrows([ExcP = MoveTemp(ExcPtr)]() { std::rethrow_exception(ExcP); });
    }

    void FErrorHandler::RethrowAndUpdateStatus()
    {
        NotifyIfThrows([]() { throw; });
    }

    template <typename FCallee> void FErrorHandler::NotifyIfThrows(FCallee Callee)
    try
    {
        Callee();
    }
    catch (const dolbyio::comms::conference_state_exception& Ex)
    {
		auto constexpr NOT_JOINED = "3";
        UpdateStatus(FMessage{"Caught dolbyio::comms::conference_state_exception: "} + Ex.what());
        if (Ex.current_state() != NOT_JOINED)
        {
            Disconnect();
        }
    }
    catch (const dolbyio::comms::invalid_token_exception& Ex)
    {
        UpdateStatus(FMessage{"Caught dolbyio::comms::invalid_token_exception: "} + Ex.what());
        Disconnect();
    }
    catch (const dolbyio::comms::dvc_error_exception& Ex)
    {
        UpdateStatus(FMessage{"Caught dolbyio::comms::dvc_error_exception: "} + Ex.what());
    }
    catch (const dolbyio::comms::peer_connection_failed_exception& Ex)
    {
        UpdateStatus(FMessage{"Caught dolbyio::comms::peer_connection_failed_exception: "} + Ex.what());
    }
    catch (const dolbyio::comms::exception& Ex)
    {
        UpdateStatus(FMessage{"Caught dolbyio::comms::exception: "} + Ex.what());
    }
    catch (const std::exception& Ex)
    {
        UpdateStatus(FMessage{"Caught std::exception: "} + Ex.what());
    }
    catch (...)
    {
        UpdateStatus("Caught unknown exception");
    }
}
