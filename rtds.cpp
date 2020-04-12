#include "rtds.h"
#include <thread>
#include "peer.h"
#include "log.h"

#ifdef RTDS_DUAL_STACK
RTDS::RTDS(unsigned short portNumber) : tcpEp(asio::ip::address_v6::any(), portNumber), tcpAcceptor(ioContext), worker(ioContext)
#else 
RTDS::RTDS(unsigned short portNumber) : tcpEp(asio::ip::address_v4::any(), portNumber), tcpAcceptor(ioContext), worker(ioContext)
#endif
{
	#ifdef PRINT_LOG
	Log::log("RTDS starting");
	#endif
}

bool RTDS::startTCPserver()
{
	tcpServerRunning = false;
	activeThreadCount = 0;

	#ifdef PRINT_LOG
	Log::log("TCP server initiating");
	#endif

	system::error_code ec;
	tcpAcceptor.open(tcpEp.protocol(), ec);
	if (ec != system::errc::success)
	{
		#if defined(PRINT_LOG) || defined(PRINT_ERROR)
		Log::log("Failed to open TCP acceptor", ec);
		#endif
		return false;
	}

	tcpAcceptor.bind(tcpEp, ec);
	if (ec != system::errc::success)
	{
		tcpAcceptor.close();
		#if defined(PRINT_LOG) || defined(PRINT_ERROR)
		Log::log("Failed to bind TCP acceptor", ec);
		#endif
		return false;
	}

	tcpAcceptor.listen(asio::socket_base::max_listen_connections, ec);
	if (ec != system::errc::success)
	{
		tcpAcceptor.close();
		#if defined(PRINT_LOG) || defined(PRINT_ERROR)
		Log::log("TCP acceptor cannot listen to port", ec);
		#endif
		return false;
	}
	startAccepting();

	#ifdef PRINT_LOG
	Log::log("TCP server started");
	#endif
	tcpServerRunning = true;
	return true;
}

bool RTDS::addThread(int threadCount)
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

void RTDS::startAccepting()
{
	#ifdef PRINT_LOG
	Log::log("TCP Accepting starting");
	#endif
	keepAccepting = true;
	_peerAcceptRoutine();
}

void RTDS::stopAccepting()
{
	keepAccepting = false;
}

void RTDS::_ioThreadJob()
{	
	system::error_code ec;
	activeThreadCount++;
	ioContext.run(ec);
	if (ec != system::errc::success)
	{
		#if defined(PRINT_LOG) || defined(PRINT_ERROR)
		Log::log("ioContext.run() failed", ec);
		#endif
	}
	activeThreadCount--;
	return;
}

void RTDS::_peerAcceptRoutine()
{
	auto peerSocket = new asio::ip::tcp::socket(ioContext);

	tcpAcceptor.async_accept(*peerSocket,
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
				peerSocket->set_option(keepAlive, ec);
				if (ec != system::errc::success)
				{
					#if defined(PRINT_LOG) || defined(PRINT_ERROR)
					Log::log("TCP set_option(keepAlive) failed : ", ec);
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
			if (keepAccepting)
				_peerAcceptRoutine();
		});
}

void RTDS::_stopIoContext()
{
	ioContext.stop();
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} while (!ioContext.stopped() && activeThreadCount == 0);
}

void RTDS::_stopTCPacceptor()
{
	system::error_code ec;
	tcpAcceptor.cancel(ec);
	if (ec != system::errc::success)
	{
		#if defined(PRINT_LOG) || defined(PRINT_ERROR)
		Log::log("TCP acceptor cannot cancel operations", ec);
		#endif
	}

	tcpAcceptor.close(ec);
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
	ioContext.reset();
	Peer::removeAllPeers();

	#ifdef PRINT_LOG
	Log::log("TCP server stopped");
	#endif

	keepAccepting = false;
	tcpServerRunning = false;
}

RTDS::~RTDS()
{
	if (tcpServerRunning)
		stopTCPserver();
	_stopIoContext();
	#ifdef PRINT_LOG
	Log::log("RTDS Exiting");
	#endif
}
