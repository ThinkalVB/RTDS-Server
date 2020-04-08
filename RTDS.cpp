#include "RTDS.h"
#include <thread>
#include "Log.h"

RTDS::RTDS(int maxTCPconn) : tcpEp(asio::ip::address_v6::any(), RTDS_PORT), tcpAcceptor(ioContext), worker(ioContext)
{
	#ifdef PRINT_LOG
	Log::log("RTDS starting");
	#endif
	peerHandler.reserve(maxTCPconn);
	threadCount = 0;
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
	Log::log("TCP server started");
	#endif
	TCPserverRunning = true;
	return true;
}

bool RTDS::addThread(int threadCount)
{
	for (int i = 0; i < threadCount; i++)
	{
		try {
			std::thread ioThread(&RTDS::_ioThreadJob, this);
			ioThread.detach();
			threadCount++;
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

bool RTDS::startTCPaccepting()
{
	try {
		std::thread ioThread(&RTDS::_acceptThreadJob, this);
		ioThread.detach();
		threadCount++;
		#ifdef PRINT_LOG
		Log::log("New Acceptor Thread added");
		#endif
	}
	catch (std::runtime_error& ec)
	{
		#ifdef PRINT_LOG
		Log::log("Cannot spawn Acceptor Thread", ec);
		#endif
		return false;
	}
	return false;
}

void RTDS::_ioThreadJob()
{	
	system::error_code ec;
	ioContext.run(ec);
	if (ec != system::errc::success)
	{
		#ifdef PRINT_LOG
		Log::log("ioContext.run() failed", ec);
		#endif
	}
	threadCount--;
	return;
}

void RTDS::_acceptThreadJob()
{
	#ifdef PRINT_LOG
	Log::log("TCP Accepting started");
	#endif

	system::error_code ec;
	boost::asio::socket_base::keep_alive keepAlive(true);

	TCPaccepting = true;
	while (TCPserverRunning)
	{
		auto peerSocket = new asio::ip::tcp::socket(ioContext);
		tcpAcceptor.accept(*peerSocket, ec);
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
			peerSocket->set_option(keepAlive);
			auto peer = new Peer(peerSocket);
			peerHandler.push_back(peer);
		}
	}
	TCPaccepting = false;
	threadCount--;

	#ifdef PRINT_LOG
	Log::log("TCP Accepting stopped");
	#endif
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
	TCPserverRunning = false;
	return true;
}

RTDS::~RTDS()
{
	if (TCPserverRunning)
		stopTCPserver();
	ioContext.stop();
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	} while (!ioContext.stopped() && threadCount == 0);

	#ifdef PRINT_LOG
	Log::log("RTDS Exiting");
	#endif
}
