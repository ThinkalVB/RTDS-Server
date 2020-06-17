#include <thread>
#include "rtds_settings.h"
#include "rtds.h"
#include "peer.h"
#include "log.h"

#ifdef RTDS_DUAL_STACK
RTDS::RTDS(unsigned short portNumber) : _tcpEp(asio::ip::address_v6::any(), portNumber), _tcpAcceptor(_ioContext), _worker(_ioContext)
#else 
RTDS::RTDS(unsigned short portNumber) : _tcpEp(asio::ip::address_v4::any(), portNumber), _tcpAcceptor(_ioContext), _worker(_ioContext)
#endif
{
	START_LOG
	DEBUG_LOG(Log::log("............... RTDS Log ..............");)
	DEBUG_LOG(Log::log("RTDS Port : ", portNumber);)
}

bool RTDS::startTCPserver(const int threadCount)
{
	DEBUG_LOG(Log::log("TCP server initiating");)
	_tcpServerRunning = false;
	_activeThreadCount = 0;
	addThread(threadCount);

	asio::error_code ec;
	_tcpAcceptor.open(_tcpEp.protocol(), ec);
	if (ec)
	{
		LOG(Log::log("Failed to open TCP acceptor - ", ec.message());)
		REGISTER_SOCKET_ERR
		return false;
	}
	DEBUG_LOG(Log::log("TCP acceptor open");)

	_tcpAcceptor.bind(_tcpEp, ec);
	if (ec)
	{
		_tcpAcceptor.close();
		LOG(Log::log("Failed to bind TCP acceptor - ", ec.message());)
		REGISTER_SOCKET_ERR
		return false;
	}
	DEBUG_LOG(Log::log("TCP acceptor binded to endpoint");)

	_tcpAcceptor.listen(asio::socket_base::max_listen_connections, ec);
	if (ec)
	{
		_tcpAcceptor.close();
		LOG(Log::log("TCP acceptor cannot listen to port - ", ec.message());)
		REGISTER_SOCKET_ERR
		return false;
	}
	DEBUG_LOG(Log::log("TCP acceptor listening to port");)
	startAccepting();
	_tcpServerRunning = true;

	DEBUG_LOG(Log::log("TCP server started");)
	return true;
}

bool RTDS::addThread(const int threadCount)
{
	for (int i = 0; i < threadCount; i++)
	{
		try {
			std::thread ioThread(&RTDS::_ioThreadJob, this);
			ioThread.detach();
			DEBUG_LOG(Log::log("New thread added to ioContext");)
		}
		catch (std::runtime_error& ec)
		{
			LOG(Log::log("Cannot spawn IO Thread - ", ec.what());)
			REGISTER_IO_ERR
			return false;
		}
	}
	return true;
}

void RTDS::addThisThread()
{
	DEBUG_LOG(Log::log("Calling thread added to ioContext");)
	_ioThreadJob();
}

int RTDS::threadCount()
{
	return _activeThreadCount;
}

void RTDS::startAccepting()
{
	DEBUG_LOG(Log::log("TCP Accepting starting");)
	_keepAccepting = true;
	_peerAcceptRoutine();
}

void RTDS::stopAccepting()
{
	DEBUG_LOG(Log::log("TCP Accepting stopped");)
	_keepAccepting = false;
	_tcpAcceptor.cancel();
}

bool RTDS::isAccepting()
{
	return _keepAccepting;
}

void RTDS::stopTCPserver()
{
	_stopTCPacceptor();
	_ioContext.reset();
	DEBUG_LOG(Log::log("TCP server stopped");)

	_keepAccepting = false;
	_tcpServerRunning = false;
}

void RTDS::_ioThreadJob()
{
	asio::error_code ec;
	_activeThreadCount++;
	_ioContext.run(ec);
	if (ec)
	{	
		LOG(Log::log("ioContext.run() failed - ", ec.message());)
		REGISTER_IO_ERR
	}

	_activeThreadCount--;
	DEBUG_LOG(Log::log("ioContext thread exiting");)
	return;
}

void RTDS::_peerAcceptRoutine()
{
	auto peerSocket = new asio::ip::tcp::socket(_ioContext);

	_tcpAcceptor.async_accept(*peerSocket,
		[peerSocket, this](asio::error_code ec)
		{
			if (ec)
			{
				peerSocket->close();
				delete peerSocket;
			}
			else
			{
				asio::socket_base::keep_alive keepAlive(true);
				asio::socket_base::enable_connection_aborted connAbortSignal(true);
				peerSocket->set_option(keepAlive, ec);
				if (ec)
				{	
					DEBUG_LOG(Log::log("TCP set_option(keepAlive) failed - ", ec.message());)			
					REGISTER_WARNING
				}

				peerSocket->set_option(connAbortSignal, ec);
				if (ec)
				{	
					DEBUG_LOG(Log::log("TCP set_option(connAbortSignal) failed - ", ec.message());)		
					REGISTER_WARNING
				}

				try {
					new Peer(peerSocket);
				}
				catch (std::bad_alloc) {
					LOG(Log::log("Peer memmory bad allocation");)
					REGISTER_MEMMORY_ERR
					peerSocket->close();
					delete peerSocket;
				}
			}
			if (_keepAccepting)
				_peerAcceptRoutine();
		});
}

void RTDS::_stopIoContext()
{
	_ioContext.stop();
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} while (!_ioContext.stopped() && _activeThreadCount == 0);
	DEBUG_LOG(Log::log("IO Context stopped");)
}

void RTDS::_stopTCPacceptor()
{
	asio::error_code ec;
	_tcpAcceptor.cancel(ec);
	if (ec)
	{	
		LOG(Log::log("TCP acceptor cannot cancel operations - ", ec.message());)	
		REGISTER_SOCKET_ERR
	}

	_tcpAcceptor.close(ec);
	if (ec)
	{	
		LOG(Log::log("TCP acceptor cannot close - ", ec.message());)	
		REGISTER_SOCKET_ERR
	}
	DEBUG_LOG(Log::log("TCP acceptor stopped");)
}

RTDS::~RTDS()
{
	if (_tcpServerRunning)
		stopTCPserver();
	_stopIoContext();

	DEBUG_LOG(Log::log("RTDS Exiting [Logging stopped]");)
	STOP_LOG
}