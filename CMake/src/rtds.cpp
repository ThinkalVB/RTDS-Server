#include "rtds.h"
#include <thread>
#include "udp_peer.h"
#include "cmd_processor.h"
#include "advanced_buffer.h"
#include "tcp_peer.h"
#include "ssl_peer.h"
#include "log.h"

#ifdef RTDS_DUAL_STACK
RTDS::RTDS(const unsigned short portNumber, const unsigned short ccmPort, short threadCount) : mTCPep(asio::ip::tcp::v6(), portNumber),
mUDPep(asio::ip::udp::v6(), portNumber), mUDPsock(mIOcontext), mTCPacceptor(mIOcontext), mTCPworker(mIOcontext), 
mSSLcontext(asio::ssl::context::sslv23), mSSLep(asio::ip::tcp::v6(), ccmPort), mSSLacceptor(mIOcontext)
#else 
RTDS::RTDS(const unsigned short portNumber, const unsigned short ccmPort, short threadCount) : mTCPep(asio::ip::tcp::v4(), portNumber),
mUDPep(asio::ip::udp::v4(), portNumber), mUDPsock(mIOcontext), mTCPacceptor(mIOcontext), mTCPworker(mIOcontext), 
mSSLcontext(asio::ssl::context::sslv23), mSSLep(asio::ip::tcp::v4(), ccmPort), mSSLacceptor(mIOcontext)
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
	mStartServer();
}

RTDS::~RTDS()
{
	if (mServerRunning)
		mStopServer();
	mCloseSockets();
	mIOcontext.stop();
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} while (!mIOcontext.stopped() && mThreadCount == 0);

	DEBUG_LOG(Log::log("RTDS Exiting [Logging stopped]");)
	STOP_LOG
}

void RTDS::mStartServer()
{
	mServerRunning = true;
	try {
		std::thread ioThreadLR(&RTDS::mUDPlistenRoutine, this);
		std::thread ioThreadAR(&RTDS::mTCPacceptRoutine, this);
		std::thread ioThreadSR(&RTDS::mSSLacceptRoutine, this);
		ioThreadLR.detach();
		ioThreadAR.detach();
		ioThreadSR.detach();
		DEBUG_LOG(Log::log("New thread to UDP, TCP & SSL routines");)
	}
	catch (std::runtime_error& ec)
	{
		LOG(Log::log("Cannot spawn IO Thread - ", ec.what());)
		exit(0);
	}
}

void RTDS::mStopServer()
{
	mServerRunning = false;
	mStopTCPserver();
	mStopUDPserver();
	mStopSSLserver();
}

void RTDS::mConfigTCPserver()
{
	asio::error_code ec;
	mTCPacceptor.open(mTCPep.protocol(), ec);
	if (ec)
	{
		LOG(Log::log("Failed to open TCP acceptor - ", ec.message());)
		exit(0);
	}
	DEBUG_LOG(Log::log("TCP acceptor open");)

	mTCPacceptor.bind(mTCPep, ec);
	if (ec)
	{
		mTCPacceptor.close();
		LOG(Log::log("Failed to bind TCP acceptor - ", ec.message());)
	}
	DEBUG_LOG(Log::log("TCP acceptor binded to endpoint");)

	mTCPacceptor.listen(asio::socket_base::max_listen_connections, ec);
	if (ec)
	{
		mTCPacceptor.close();
		LOG(Log::log("TCP acceptor cannot listen to port - ", ec.message());)
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
		exit(0);
	}
	DEBUG_LOG(Log::log("UDP sock open");)

	mUDPsock.bind(mUDPep, ec);
	if (ec)
	{
		mUDPsock.close();
		LOG(Log::log("Failed to bind UDP socket - ", ec.message());)
		exit(0);
	}
	DEBUG_LOG(Log::log("UDP socket binded to endpoint");)
}

void RTDS::mConfigSSLserver()
{
	asio::error_code ec;
	mSSLcontext.set_options(asio::ssl::context::default_workarounds
		| asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use, ec);
	if (ec)
	{
		LOG(Log::log("Failed to set SSL options - ", ec.message());)
		exit(0);
	}

	mSSLcontext.use_certificate_file("server_cert.pem", asio::ssl::context::pem, ec);
	if (ec)
	{
		LOG(Log::log("Failed to use SSL certificate - ", ec.message());)
		exit(0);
	}

	mSSLcontext.use_private_key_file("server_cert.pem", asio::ssl::context::pem, ec);
	if (ec)
	{
		LOG(Log::log("Failed to set SSL Private key - ", ec.message());)
		exit(0);
	}

	mSSLcontext.use_tmp_dh_file("dh2048.pem", ec);
	if (ec)
	{
		LOG(Log::log("Failed to DH512 - ", ec.message());)
		exit(0);
	}

	mSSLacceptor.open(mTCPep.protocol(), ec);
	if (ec)
	{
		LOG(Log::log("Failed to open SSL acceptor - ", ec.message());)
		exit(0);
	}
	DEBUG_LOG(Log::log("SSL acceptor open");)

	mSSLacceptor.bind(mSSLep, ec);
	if (ec)
	{
		mTCPacceptor.close();
		LOG(Log::log("Failed to bind SSL acceptor - ", ec.message());)
	}
	DEBUG_LOG(Log::log("SSL acceptor binded to endpoint");)

	mSSLacceptor.listen(asio::socket_base::max_listen_connections, ec);
	if (ec)
	{
		mSSLacceptor.close();
		LOG(Log::log("SSL acceptor cannot listen to port - ", ec.message());)
		exit(0);
	}
	DEBUG_LOG(Log::log("SSL acceptor listening to port");)
}

void RTDS::mIOthreadJob()
{
	asio::error_code ec;
	mThreadCount++;
	mIOcontext.run(ec);
	if (ec)
	{	LOG(Log::log("ioContext.run() failed - ", ec.message());)	}

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
			continue;
		}

		asio::error_code ec;
		mTCPacceptor.accept(*peerSocket, ec);
		if (ec)
		{
			if (mServerRunning) {
				DEBUG_LOG(Log::log("TCP Acceptor failed - ", ec.message());)
			}
			delete peerSocket;
		}
		else
		{
			asio::socket_base::keep_alive keepAlive(true);
			asio::socket_base::enable_connection_aborted connAbortSignal(true);
			peerSocket->set_option(keepAlive, ec);
			if (ec)
			{	DEBUG_LOG(Log::log("TCP set_option(keepAlive) failed - ", ec.message());)	}

			peerSocket->set_option(connAbortSignal, ec);
			if (ec)
			{	DEBUG_LOG(Log::log("TCP set_option(connAbortSignal) failed - ", ec.message());)	}

			auto peer = new (std::nothrow) TCPpeer(peerSocket);
			if (peer == nullptr)
			{
				LOG(Log::log("TCP Peer memmory bad allocation");)
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
	UDPpeer udpPeer(&mUDPsock);
	asio::error_code ec;

	while (mServerRunning)
	{
		auto dataSize = mUDPsock.receive_from(udpPeer.getReadBuffer(), udpPeer.getRefToEndpoint(), 0, ec);
		if (ec)
		{	
			if (mServerRunning) {
				DEBUG_LOG(Log::log("UDP receive failed - ", ec.message());)
			}
		}
		else
		{
			if (udpPeer.cookString(dataSize))
				CmdProcessor::processCommand(udpPeer);
			else
				udpPeer.respondWith(Response::BAD_COMMAND);
		}
	}
	mThreadCount--;
}

void RTDS::mSSLacceptRoutine()
{
	mThreadCount++;
	while (mServerRunning)
	{
		auto peerSocket = new (std::nothrow) SSLsocket(mIOcontext, mSSLcontext);
		if (peerSocket == nullptr)
		{
			LOG(Log::log("SSL Peer socket bad allocation");)
			std::this_thread::sleep_for(std::chrono::seconds(1));
			continue;
		}

		asio::error_code ec;
		mSSLacceptor.accept((*peerSocket).lowest_layer(), ec);
		if (ec)
		{
			if (mServerRunning) {
				DEBUG_LOG(Log::log("SSL Acceptor failed - ", ec.message());)
			}
			delete peerSocket;
		}
		else
		{
			asio::socket_base::keep_alive keepAlive(true);
			asio::socket_base::enable_connection_aborted connAbortSignal(true);
			peerSocket->lowest_layer().set_option(keepAlive, ec);
			if (ec)
			{	DEBUG_LOG(Log::log("SSL set_option(keepAlive) failed - ", ec.message());)	}

			peerSocket->lowest_layer().set_option(connAbortSignal, ec);;
			if (ec)
			{	DEBUG_LOG(Log::log("SSL set_option(connAbortSignal) failed - ", ec.message());)	}

			peerSocket->handshake(asio::ssl::stream_base::server, ec);
			if (ec)
			{	DEBUG_LOG(Log::log("SSL handshake failed - ", ec.message());)	}

			auto peer = new (std::nothrow) SSLpeer(peerSocket);
			if (peer == nullptr)
			{
				LOG(Log::log("Peer memmory bad allocation");)
				peerSocket->shutdown();
				delete peerSocket;
			}
		}
	}
	mThreadCount--;
}

void RTDS::mStopUDPserver()
{
	asio::error_code ec;
	mUDPsock.cancel(ec);
	if (ec)
	{	DEBUG_LOG(Log::log("UDP socket cannot cancel - ", ec.message());)	}
}

void RTDS::mStopTCPserver()
{
	asio::error_code ec;
	mTCPacceptor.cancel(ec);
	if (ec)
	{	DEBUG_LOG(Log::log("TCP acceptor cannot cancel - ", ec.message());)	}
}

void RTDS::mStopSSLserver()
{
	asio::error_code ec;
	mSSLacceptor.cancel(ec);
	if (ec)
	{	DEBUG_LOG(Log::log("SSL acceptor cannot cancel - ", ec.message());)	}
}

void RTDS::mCloseSockets()
{
	asio::error_code ec;
	mUDPsock.close(ec);
	if (ec)
	{	DEBUG_LOG(Log::log("UDP socket cannot close - ", ec.message());)	}

	mTCPacceptor.close(ec);
	if (ec)
	{	DEBUG_LOG(Log::log("UDP socket cannot close - ", ec.message());)	}

	mSSLacceptor.close(ec);
	if (ec)
	{	DEBUG_LOG(Log::log("SSL socket cannot close - ", ec.message());)	}
}
