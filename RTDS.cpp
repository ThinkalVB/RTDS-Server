#include "RTDS.h"
#include <thread>
#include "Log.h"

#ifdef RTDS_DUAL_STACK
RTDS::RTDS(unsigned short portNumber, int maxTCPconn) : tcpEp(asio::ip::address_v6::any(), portNumber), tcpAcceptor(ioContext), worker(ioContext)
#else 
RTDS::RTDS(unsigned short portNumber, int maxTCPconn) : tcpEp(asio::ip::address_v4::any(), portNumber), tcpAcceptor(ioContext), worker(ioContext)
#endif
{
	#ifdef PRINT_LOG
	Log::log("RTDS starting");
	#endif

	peerHandler.reserve(maxTCPconn);
	activeThreadCount = 0;
}

bool RTDS::startTCPserver()
{
	#ifdef PRINT_LOG
	Log::log("TCP server initiating");
	#endif

	system::error_code ec;
	tcpAcceptor.open(tcpEp.protocol(), ec);
	if (ec != system::errc::success)
	{
		#ifdef PRINT_LOG
		Log::log("Failed to open TCP acceptor", ec);
		#endif
		return false;
	}

	tcpAcceptor.bind(tcpEp, ec);
	if (ec != system::errc::success)
	{
		tcpAcceptor.close();
		#ifdef PRINT_LOG
		Log::log("Failed to bind TCP acceptor", ec);
		#endif
		return false;
	}

	tcpAcceptor.listen(asio::socket_base::max_listen_connections, ec);
	if (ec != system::errc::success)
	{
		tcpAcceptor.close();
		#ifdef PRINT_LOG
		Log::log("TCP acceptor cannot listen to port", ec);
		#endif
		return false;
	}

	#ifdef PRINT_LOG
	Log::log("TCP Accepting starting");
	#endif
	_keepAccepting = true;
	_peerAcceptRoutine();

	#ifdef PRINT_LOG
	Log::log("TCP server started");
	#endif
	_tcpServerRunning = true;
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
			#ifdef PRINT_LOG
			Log::log("Cannot spawn IO Thread", ec);
			#endif
			return false;
		}
	}
	return true;
}

void RTDS::_ioThreadJob()
{	
	system::error_code ec;
	activeThreadCount++;
	ioContext.run(ec);
	if (ec != system::errc::success)
	{
		#ifdef PRINT_LOG
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
					#ifdef PRINT_LOG
					Log::log("TCP set_option(keepAlive) failed : ", ec);
					#endif
				}

				auto peer = new Peer(peerSocket);
				peerHandler.push_back(peer);
			}
			if (_keepAccepting)
				_peerAcceptRoutine();
		});
}

bool RTDS::stopTCPserver()
{
	system::error_code ec;
	tcpAcceptor.cancel(ec);
	if (ec != system::errc::success)
	{
		#ifdef PRINT_LOG
		Log::log("TCP acceptor cannot cancel operations", ec);
		#endif
		return false;
	}

	tcpAcceptor.close(ec);
	if (ec != system::errc::success)
	{
		#ifdef PRINT_LOG
		Log::log("TCP acceptor cannot close", ec);
		#endif
		return false;
	}

	for (int i = 0; i < peerHandler.size(); i++)
		delete peerHandler[i];
	peerHandler.clear();

	#ifdef PRINT_LOG
	Log::log("TCP server stopped");
	#endif

	_keepAccepting = false;
	_tcpServerRunning = false;
	return true;
}

RTDS::~RTDS()
{
	if (_tcpServerRunning)
		stopTCPserver();
	ioContext.stop();
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	} while (!ioContext.stopped() && activeThreadCount == 0);

	#ifdef PRINT_LOG
	Log::log("RTDS Exiting");
	#endif
}
