#include "rtds.h"
#include <thread>
#include "peer.h"
#include "log.h"

#ifdef RTDS_DUAL_STACK
RTDS::RTDS(unsigned short portNumber) : _tcpEp(asio::ip::address_v6::any(), portNumber), _tcpAcceptor(_ioContext), _worker(_ioContext)
#else 
RTDS::RTDS(unsigned short portNumber) : _tcpEp(asio::ip::address_v4::any(), portNumber), _tcpAcceptor(_ioContext), _worker(_ioContext)
#endif
{
	#if defined(PRINT_LOG) || defined(PRINT_ERROR)
	Log::startLog("logs.txt");
	#endif

	#ifdef PRINT_LOG
	Log::log("RTDS starting");
	#endif
}

bool RTDS::startTCPserver()
{
	_tcpServerRunning = false;
	_activeThreadCount = 0;

	#ifdef PRINT_LOG
	Log::log("TCP server initiating");
	#endif

	system::error_code ec;
	_tcpAcceptor.open(_tcpEp.protocol(), ec);
	if (ec != system::errc::success)
	{
		#if defined(PRINT_LOG) || defined(PRINT_ERROR)
		Log::log("Failed to open TCP acceptor", ec);
		#endif
		return false;
	}

	_tcpAcceptor.bind(_tcpEp, ec);
	if (ec != system::errc::success)
	{
		_tcpAcceptor.close();
		#if defined(PRINT_LOG) || defined(PRINT_ERROR)
		Log::log("Failed to bind TCP acceptor", ec);
		#endif
		return false;
	}

	_tcpAcceptor.listen(asio::socket_base::max_listen_connections, ec);
	if (ec != system::errc::success)
	{
		_tcpAcceptor.close();
		#if defined(PRINT_LOG) || defined(PRINT_ERROR)
		Log::log("TCP acceptor cannot listen to port", ec);
		#endif
		return false;
	}
	startAccepting();

	#ifdef PRINT_LOG
	Log::log("TCP server started");
	#endif
	_tcpServerRunning = true;
	return true;
}

bool RTDS::addThread(const int threadCount)
{
	for (int i = 0; i < threadCount; i++)
	{
		try {
			std::thread ioThread(&RTDS::_ioThreadJob, this);
			ioThread.detach();
			#ifdef PRINT_LOG
			Log::log("New IO Thread added");
			#endif

		}
		catch (std::runtime_error& ec)
		{
			#if defined(PRINT_LOG) || defined(PRINT_ERROR)
			Log::log("Cannot spawn IO Thread", ec);
			#endif
			return false;
		}
	}
	return true;
}

void RTDS::addThisThread()
{
	_ioThreadJob();
}

void RTDS::startAccepting()
{
	#ifdef PRINT_LOG
	Log::log("TCP Accepting starting");
	#endif
	_keepAccepting = true;
	_peerAcceptRoutine();
}

void RTDS::stopAccepting()
{
	_keepAccepting = false;
	_tcpAcceptor.cancel();
}

void RTDS::_ioThreadJob()
{	
	system::error_code ec;
	_activeThreadCount++;
	_ioContext.run(ec);
	if (ec != system::errc::success)
	{
		#if defined(PRINT_LOG) || defined(PRINT_ERROR)
		Log::log("ioContext.run() failed", ec);
		#endif
	}
	_activeThreadCount--;
	return;
}

void RTDS::_peerAcceptRoutine()
{
	auto peerSocket = new asio::ip::tcp::socket(_ioContext);

	_tcpAcceptor.async_accept(*peerSocket,
		[peerSocket, this](boost::system::error_code ec)
		{
			if (ec != system::errc::success)
			{
				peerSocket->close();
				delete peerSocket;
			}
			else
			{
				#ifdef PRINT_LOG
				Log::log("Incoming TCP connection", peerSocket);
				#endif

				boost::asio::socket_base::keep_alive keepAlive(true);
				asio::socket_base::enable_connection_aborted connAbortSignal(true);
				peerSocket->set_option(keepAlive, ec);
				if (ec != system::errc::success)
				{
					#if defined(PRINT_LOG) || defined(PRINT_ERROR)
					Log::log("TCP set_option(keepAlive) failed : ", ec);
					#endif
				}
				peerSocket->set_option(connAbortSignal, ec);
				if (ec != system::errc::success)
				{
					#if defined(PRINT_LOG) || defined(PRINT_ERROR)
					Log::log("TCP set_option(connAbortSignal) failed : ", ec);
					#endif
				}

				try
				{
					auto peer = new Peer(peerSocket);
				}
				catch (std::bad_alloc)
				{
					#if defined(PRINT_LOG) || defined(PRINT_ERROR)
					Log::log("Peer memmory bad allocation");
					#endif
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
}

void RTDS::_stopTCPacceptor()
{
	system::error_code ec;
	_tcpAcceptor.cancel(ec);
	if (ec != system::errc::success)
	{
		#if defined(PRINT_LOG) || defined(PRINT_ERROR)
		Log::log("TCP acceptor cannot cancel operations", ec);
		#endif
	}

	_tcpAcceptor.close(ec);
	if (ec != system::errc::success)
	{
		#if defined(PRINT_LOG) || defined(PRINT_ERROR)
		Log::log("TCP acceptor cannot close", ec);
		#endif
	}
}

void RTDS::stopTCPserver()
{
	_stopTCPacceptor();
	_ioContext.reset();

	#ifdef PRINT_LOG
	Log::log("TCP server stopped");
	#endif

	_keepAccepting = false;
	_tcpServerRunning = false;
}

short RTDS::getPeerCount()
{
	return Peer::getPeerCount();
}

RTDS::~RTDS()
{
	if (_tcpServerRunning)
		stopTCPserver();
	_stopIoContext();

	#ifdef PRINT_LOG
	Log::log("RTDS Exiting");
	#endif

	#if defined(PRINT_LOG) || defined(PRINT_ERROR)
	Log::stopLog();
	#endif
}