#include "rtds.h"
#include "peer.h"
#include <thread>
#include "log.h"

#ifdef RTDS_DUAL_STACK
RTDS::RTDS(unsigned short portNumber) : _tcpEp(asio::ip::address_v6::any(), portNumber), _tcpAcceptor(_ioContext), _worker(_ioContext)
#else 
RTDS::RTDS(unsigned short portNumber) : _tcpEp(asio::ip::address_v4::any(), portNumber), _tcpAcceptor(_ioContext), _worker(_ioContext)
#endif
{
	START_LOG
	DEBUG_LOG(Log::ALog("............... RTDS Log ..............");)
	DEBUG_LOG(Log::ALog("RTDS Port : ", portNumber);)
}

bool RTDS::startTCPserver(const int threadCount)
{
	DEBUG_LOG(Log::ALog("TCP server initiating");)
	_tcpServerRunning = false;
	_activeThreadCount = 0;
	addThread(threadCount);

	asio::error_code ec;
	_tcpAcceptor.open(_tcpEp.protocol(), ec);
	if (ec)
	{
		LOG(Log::log("Failed to open TCP acceptor", ec);)
		return false;
	}
	DEBUG_LOG(Log::ALog("TCP acceptor open");)

	_tcpAcceptor.bind(_tcpEp, ec);
	if (ec)
	{
		_tcpAcceptor.close();
		LOG(Log::log("Failed to bind TCP acceptor", ec);)
		return false;
	}
	DEBUG_LOG(Log::ALog("TCP acceptor binded to endpoint");)

	_tcpAcceptor.listen(asio::socket_base::max_listen_connections, ec);
	if (ec)
	{
		_tcpAcceptor.close();
		LOG(Log::log("TCP acceptor cannot listen to port", ec);)
		return false;
	}
	DEBUG_LOG(Log::ALog("TCP acceptor listening to port");)
	startAccepting();
	_tcpServerRunning = true;

	DEBUG_LOG(Log::ALog("TCP server started");)
	return true;
}

bool RTDS::addThread(const int threadCount)
{
	for (int i = 0; i < threadCount; i++)
	{
		try {
			std::thread ioThread(&RTDS::_ioThreadJob, this);
			ioThread.detach();
			DEBUG_LOG(Log::ALog("New thread added to ioContext");)
		}
		catch (std::runtime_error& ec)
		{
			LOG(Log::log("Cannot spawn IO Thread", ec);)
			return false;
		}
	}
	return true;
}

void RTDS::addThisThread()
{
	DEBUG_LOG(Log::ALog("Calling thread added to ioContext");)
	_ioThreadJob();
}

void RTDS::startAccepting()
{
	DEBUG_LOG(Log::ALog("TCP Accepting starting");)
	_keepAccepting = true;
	_peerAcceptRoutine();
}

void RTDS::stopAccepting()
{
	DEBUG_LOG(Log::ALog("TCP Accepting stopped");)
	_keepAccepting = false;
	_tcpAcceptor.cancel();
}

void RTDS::stopTCPserver()
{
	_stopTCPacceptor();
	_ioContext.reset();
	DEBUG_LOG(Log::ALog("TCP server stopped");)

	_keepAccepting = false;
	_tcpServerRunning = false;
}

void RTDS::_ioThreadJob()
{
	asio::error_code ec;
	_activeThreadCount++;
	_ioContext.run(ec);
	if (ec)
		LOG(Log::log("ioContext.run() failed", ec);)

	_activeThreadCount--;
	DEBUG_LOG(Log::ALog("ioContext thread exiting");)
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
				DEBUG_LOG(Log::log("Incoming TCP connection", peerSocket);)
				asio::socket_base::keep_alive keepAlive(true);
				asio::socket_base::enable_connection_aborted connAbortSignal(true);
				peerSocket->set_option(keepAlive, ec);
				if (ec)
					DEBUG_LOG(Log::log("TCP set_option(keepAlive) failed : ", ec);)

				peerSocket->set_option(connAbortSignal, ec);
				if (ec)
					DEBUG_LOG(Log::log("TCP set_option(connAbortSignal) failed : ", ec);)

				try {
					new Peer(peerSocket);
				}
				catch (std::bad_alloc) {
					LOG(Log::ALog("Peer memmory bad allocation");)
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
	DEBUG_LOG(Log::ALog("IO Context stopped");)
}

void RTDS::_stopTCPacceptor()
{
	asio::error_code ec;
	_tcpAcceptor.cancel(ec);
	if (ec)
		LOG(Log::log("TCP acceptor cannot cancel operations", ec);)

	_tcpAcceptor.close(ec);
	if (ec)
		LOG(Log::log("TCP acceptor cannot close", ec);)
	DEBUG_LOG(Log::ALog("TCP acceptor stopped");)
}

RTDS::~RTDS()
{
	if (_tcpServerRunning)
		stopTCPserver();
	_stopIoContext();

	DEBUG_LOG(Log::ALog("RTDS Exiting [Logging stopped]");)
	STOP_LOG
}