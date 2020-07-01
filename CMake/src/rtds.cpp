#include "rtds.h"
#include <thread>
#include "rtds_ccm.h"
#include "udp_peer.h"
#include "cmd_processor.h"
#include "advanced_buffer.h"
#include "tcp_peer.h"
#include "log.h"

typedef asio::ssl::stream<asio::ip::tcp::socket> SSLsocket;

#ifdef RTDS_DUAL_STACK
RTDS::RTDS(const unsigned short portNumber, const unsigned short ccmPort, short threadCount) : mTCPep(asio::ip::address_v6::any(), portNumber),
mUDPep(asio::ip::address_v6::any(), portNumber), mUDPsock(mIOcontext), mTCPacceptor(mIOcontext), mTCPworker(mIOcontext), 
mSSLcontext(asio::ssl::context::sslv23), mSSLep(asio::ip::address_v6::any(), ccmPort), mSSLacceptor(mIOcontext)
#else 
RTDS::RTDS(const unsigned short portNumber, const unsigned short ccmPort, short threadCount) : mTCPep(asio::ip::address_v4::any(), portNumber),
mUDPep(asio::ip::address_v4::any(), portNumber), mUDPsock(mIOcontext), mTCPacceptor(mIOcontext), mTCPworker(mIOcontext), 
mSSLcontext(asio::ssl::context::sslv23), mSSLep(asio::ip::address_v4::any(), ccmPort), mSSLacceptor(mIOcontext)
#endif
{
	START_LOG
	DEBUG_LOG(Log::log("............... RTDS Log ..............");)
	DEBUG_LOG(Log::log("RTDS Port : ", portNumber);)
	mThreadCount = 0;
	mAddthread(threadCount - 3);
	mConfigTCPserver();
	mConfigUDPserver();
	mConfigSSLserver();
}

RTDS::~RTDS()
{
	if (mServerRunning)
		stopServer();
	
	mCloseSockets();
	mIOcontext.stop();
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} while (!mIOcontext.stopped() && mThreadCount == 0);

	DEBUG_LOG(Log::log("RTDS Exiting [Logging stopped]");)
	STOP_LOG
}

void RTDS::startServer()
{
	mIOcontext.restart();
	mServerRunning = true;
	try {
		std::thread ioThreadLR(&RTDS::mUDPlistenRoutine, this);
		std::thread ioThreadAR(&RTDS::mTCPacceptRoutine, this);
		ioThreadLR.detach();
		ioThreadAR.detach();
		DEBUG_LOG(Log::log("New thread to UDP & TCP routines");)
	}
	catch (std::runtime_error& ec)
	{
		LOG(Log::log("Cannot spawn IO Thread - ", ec.what());)
		REGISTER_IO_ERR
		exit(0);
	}
}

void RTDS::stopServer()
{
	mServerRunning = false;
	mStopTCPserver();
	mStopUDPserver();
}

bool RTDS::isActive()
{
	return mServerRunning;
}

void RTDS::printStatus()
{
	std::cout << "Active Threads : " << mThreadCount << "\t\t";
	std::cout << "RTDS Port      : " << mTCPep.port() << std::endl;

	if (mServerRunning)
		std::cout << "RTDS Running   : OK " << "\t\t";
	else
		std::cout << "RTDS Running   : OK " << "\t\t";
	std::cout << "Connections    : " << TCPpeer::peerCount() << std::endl;

	std::cout << "Warning        : " << WARNINGS << "\t\t";
	std::cout << "Memmory Error  : " << MEMMORY_ERR << std::endl;

	std::cout << "Socket Error   : " << SOCKET_ERR << "\t\t";
	std::cout << "IO Error       : " << IO_ERR << std::endl;
	std::cout << "Code Error     : " << CODE_ERR << std::endl;
}

void RTDS::mConfigTCPserver()
{
	asio::error_code ec;
	mTCPacceptor.open(mTCPep.protocol(), ec);
	if (ec)
	{
		LOG(Log::log("Failed to open TCP acceptor - ", ec.message());)
		REGISTER_SOCKET_ERR
		exit(0);
	}
	DEBUG_LOG(Log::log("TCP acceptor open");)

	mTCPacceptor.bind(mTCPep, ec);
	if (ec)
	{
		mTCPacceptor.close();
		LOG(Log::log("Failed to bind TCP acceptor - ", ec.message());)
		REGISTER_SOCKET_ERR

	}
	DEBUG_LOG(Log::log("TCP acceptor binded to endpoint");)

	mTCPacceptor.listen(asio::socket_base::max_listen_connections, ec);
	if (ec)
	{
		mTCPacceptor.close();
		LOG(Log::log("TCP acceptor cannot listen to port - ", ec.message());)
		REGISTER_SOCKET_ERR
		exit(0);
	}
	DEBUG_LOG(Log::log("TCP acceptor listening to port");)
}

void RTDS::mConfigUDPserver()
{
	asio::error_code ec;
	mUDPsock.open(mUDPep.protocol(), ec);
	if (ec)
	{
		LOG(Log::log("Failed to open UDP acceptor - ", ec.message());)
		REGISTER_SOCKET_ERR
		exit(0);
	}
	DEBUG_LOG(Log::log("UDP sock open");)

	mUDPsock.bind(mUDPep, ec);
	if (ec)
	{
		mUDPsock.close();
		LOG(Log::log("Failed to bind UDP socket - ", ec.message());)
		REGISTER_SOCKET_ERR
		exit(0);
	}
	UDPpeer::registerUDPsocket(&mUDPsock);
	DEBUG_LOG(Log::log("UDP socket binded to endpoint");)
}

void RTDS::mConfigSSLserver()
{
	asio::error_code ec;
	mSSLcontext.set_options(asio::ssl::context::default_workarounds, ec);
	if (ec)
	{
		LOG(Log::log("Failed to set SSL options - ", ec.message());)
		REGISTER_SOCKET_ERR
		exit(0);
	}

	mSSLcontext.use_certificate_file("RTDScert.pem", asio::ssl::context::pem, ec);
	mSSLcontext.set_options(asio::ssl::context::default_workarounds, ec);
	if (ec)
	{
		LOG(Log::log("Failed to use SSL certificate - ", ec.message());)
		REGISTER_SOCKET_ERR
		exit(0);
	}

	mSSLcontext.use_private_key_file("RTDSpk.pem", asio::ssl::context::pem, ec);
	mSSLcontext.set_options(asio::ssl::context::default_workarounds, ec);
	if (ec)
	{
		LOG(Log::log("Failed to set SSL Private key - ", ec.message());)
		REGISTER_SOCKET_ERR
		exit(0);
	}

	mSSLacceptor.open(mTCPep.protocol(), ec);
	if (ec)
	{
		LOG(Log::log("Failed to open SSL acceptor - ", ec.message());)
		REGISTER_SOCKET_ERR
		exit(0);
	}
	DEBUG_LOG(Log::log("SSL acceptor open");)

	mSSLacceptor.bind(mSSLep, ec);
	if (ec)
	{
		mTCPacceptor.close();
		LOG(Log::log("Failed to bind SSL acceptor - ", ec.message());)
			REGISTER_SOCKET_ERR

	}
	DEBUG_LOG(Log::log("SSL acceptor binded to endpoint");)

	mSSLacceptor.listen(asio::socket_base::max_listen_connections, ec);
	if (ec)
	{
		mSSLacceptor.close();
		LOG(Log::log("SSL acceptor cannot listen to port - ", ec.message());)
		REGISTER_SOCKET_ERR
		exit(0);
	}
	DEBUG_LOG(Log::log("SSL acceptor listening to port");)

	try {
		std::thread ioThread(&RTDS::mSSLacceptRoutine, this);
		ioThread.detach();
		DEBUG_LOG(Log::log("New thread to SSL routine");)
	}
	catch (std::runtime_error& ec)
	{
		LOG(Log::log("Cannot spawn IO Thread - ", ec.what());)
		REGISTER_IO_ERR
		exit(0);
	}
}

void RTDS::mIOthreadJob()
{
	asio::error_code ec;
	mThreadCount++;
	mIOcontext.run(ec);
	if (ec)
	{
		LOG(Log::log("ioContext.run() failed - ", ec.message());)
		REGISTER_IO_ERR
	}

	mThreadCount--;
	DEBUG_LOG(Log::log("ioContext thread exiting");)
}

void RTDS::mAddthread(int threadCount)
{
	for (int i = 0; i < threadCount; i++)
	{
		try {
			std::thread ioThread(&RTDS::mIOthreadJob, this);
			ioThread.detach();
			DEBUG_LOG(Log::log("New thread added to ioContext");)
		}
		catch (std::runtime_error& ec)
		{
			LOG(Log::log("Cannot spawn IO Thread - ", ec.what());)
			REGISTER_IO_ERR
			exit(0);
		}
	}
}

void RTDS::mTCPacceptRoutine()
{
	mThreadCount++;
	while (mServerRunning)
	{
		auto peerSocket = new (std::nothrow) asio::ip::tcp::socket(mIOcontext);
		if (peerSocket == nullptr)
		{
			LOG(Log::log("TCP Peer socket bad allocation");)
			std::this_thread::sleep_for(std::chrono::seconds(1));
			REGISTER_MEMMORY_ERR
			continue;
		}

		asio::error_code ec;
		mTCPacceptor.accept(*peerSocket, ec);
		if (ec)
		{
			DEBUG_LOG(Log::log("TCP Acceptor failed - ", ec.message());)
			REGISTER_SOCKET_ERR
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

			auto peer = new (std::nothrow) TCPpeer(peerSocket);
			if (peer == nullptr)
			{
				LOG(Log::log("TCP Peer memmory bad allocation");)
				REGISTER_MEMMORY_ERR
				peerSocket->close();
				delete peerSocket;
			}
		}
	}
	mThreadCount--;
}

void RTDS::mUDPlistenRoutine()
{
	mThreadCount++;
	UDPpeer udpPeer;
	AdancedBuffer dataBuffer;
	asio::error_code ec;

	while (mServerRunning)
	{
		auto dataSize = mUDPsock.receive_from(dataBuffer.getReadBuffer(), udpPeer.getRefToEp(), 0, ec);
		if (ec)
		{	
			DEBUG_LOG(Log::log("UDP receive failed - ", ec.message());)	
			REGISTER_SOCKET_ERR
		}
		else
		{
			dataBuffer.cookString(dataSize);
			CmdProcessor::processCommand(udpPeer, dataBuffer);
			mUDPsock.send_to(dataBuffer.getSendBuffer(), udpPeer.getRefToEp());
		}
	}
	mThreadCount--;
}

void RTDS::mSSLacceptRoutine()
{
	mThreadCount++;
	while (true)
	{
		auto peerSocket = new (std::nothrow) SSLsocket(mIOcontext, mSSLcontext);
		if (peerSocket == nullptr)
		{
			LOG(Log::log("SSL Peer socket bad allocation");)
			std::this_thread::sleep_for(std::chrono::seconds(1));
			REGISTER_MEMMORY_ERR
			continue;
		}

		asio::error_code ec;
		mSSLacceptor.accept((*peerSocket).lowest_layer(), ec);
		if (ec)
		{
			DEBUG_LOG(Log::log("SSL Acceptor failed - ", ec.message());)
			REGISTER_SOCKET_ERR
			delete peerSocket;
		}
		else
		{
			asio::socket_base::keep_alive keepAlive(true);
			asio::socket_base::enable_connection_aborted connAbortSignal(true);
			peerSocket->lowest_layer().set_option(keepAlive, ec);
			if (ec)
			{
				DEBUG_LOG(Log::log("TCP set_option(keepAlive) failed - ", ec.message());)
				REGISTER_WARNING
			}

			peerSocket->lowest_layer().set_option(connAbortSignal, ec);;
			if (ec)
			{
				DEBUG_LOG(Log::log("TCP set_option(connAbortSignal) failed - ", ec.message());)
				REGISTER_WARNING
			}

			/*
			auto peer = new (std::nothrow) TCPpeer(peerSocket);
			if (peer == nullptr)
			{
				LOG(Log::log("Peer memmory bad allocation");)
				REGISTER_MEMMORY_ERR
				peerSocket->close();
				delete peerSocket;
			}
			*/
		}
	}
	mThreadCount--;
}

void RTDS::mStopUDPserver()
{
	asio::error_code ec;
	mUDPsock.cancel(ec);
	if (ec)
	{	
		DEBUG_LOG(Log::log("UDP socket cannot cancel - ", ec.message());)	
		REGISTER_SOCKET_ERR
	}
}

void RTDS::mStopTCPserver()
{
	asio::error_code ec;
	mTCPacceptor.cancel(ec);
	if (ec)
	{	
		DEBUG_LOG(Log::log("TCP acceptor cannot cancel - ", ec.message());)	
		REGISTER_SOCKET_ERR
	}
}

void RTDS::mCloseSockets()
{
	asio::error_code ec;
	mUDPsock.close(ec);
	if (ec)
	{	
		DEBUG_LOG(Log::log("UDP socket cannot close - ", ec.message());)	
		REGISTER_SOCKET_ERR
	}

	mTCPacceptor.close(ec);
	{	
		DEBUG_LOG(Log::log("UDP socket cannot close - ", ec.message());)
		REGISTER_SOCKET_ERR
	}
}
