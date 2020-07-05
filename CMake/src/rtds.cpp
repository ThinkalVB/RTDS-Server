#include "rtds.h"
#include <thread>
#include "cmd_processor.h"
#include "log.h"

#include "udp_peer.h"
#include "tcp_peer.h"
#include "ssl_ccm.h"

#ifdef RTDS_DUAL_STACK
RTDS::RTDS(const unsigned short portNumber, const unsigned short ccmPort, short threadCount) : mTCPep(asio::ip::tcp::v6(), portNumber),
mUDPep(asio::ip::udp::v6(), portNumber), mUDPsock(mIOcontext), mTCPacceptor(mIOcontext), mIOworker(mIOcontext), 
mCCMcontext(asio::ssl::context::sslv23), mCCMep(asio::ip::tcp::v6(), ccmPort), mCCMacceptor(mIOcontext)
#else 
RTDS::RTDS(const unsigned short portNumber, const unsigned short ccmPort, short threadCount) : mTCPep(asio::ip::tcp::v4(), portNumber),
mUDPep(asio::ip::udp::v4(), portNumber), mUDPsock(mIOcontext), mTCPacceptor(mIOcontext), mTCPworker(mIOcontext), 
mCCMcontext(asio::ssl::context::sslv23), mCCMep(asio::ip::tcp::v4(), ccmPort), mCCMacceptor(mIOcontext)
#endif
{
	START_LOG
	DEBUG_LOG(Log::log("............... RTDS Log ..............");)
	DEBUG_LOG(Log::log("RTDS Port : ", portNumber);)
	mThreadCount = 0;
	mAddthread(threadCount);
	mConfigTCPserver();
	mConfigUDPserver();
	mConfigCCMserver();
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
		std::thread ioThreadSR(&RTDS::mCCMacceptRoutine, this);
		ioThreadLR.detach();
		ioThreadAR.detach();
		ioThreadSR.detach();
		DEBUG_LOG(Log::log("New thread to UDP, TCP & CCM routines");)
	}
	catch (const std::runtime_error& ec)
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
	mStopCCMserver();
}

void RTDS::mConfigTCPserver()
{
	DEBUG_LOG(Log::log("TCP server configuring...");)
	try {
		mTCPacceptor.open(mTCPep.protocol());
		DEBUG_LOG(Log::log("TCP acceptor open");)
		mTCPacceptor.bind(mTCPep);
		DEBUG_LOG(Log::log("TCP acceptor bound to endpoint");)
		mTCPacceptor.listen(asio::socket_base::max_listen_connections);
		DEBUG_LOG(Log::log("TCP acceptor started listening");)
	}
	catch (const asio::error_code ec)
	{
		LOG(Log::log("Failed to configure TCP server - ", ec.message());)
		exit(0);
	}
	DEBUG_LOG(Log::log("TCP server configured");)
}

void RTDS::mConfigUDPserver()
{
	DEBUG_LOG(Log::log("UDP socket configuring...");)
	try {
		mUDPsock.open(mUDPep.protocol());
		DEBUG_LOG(Log::log("UDP acceptor open");)
		mUDPsock.bind(mUDPep);
		DEBUG_LOG(Log::log("UDP acceptor bound to endpoint");)
	}
	catch (const asio::error_code& ec)
	{
		LOG(Log::log("Failed to configure UDP server - ", ec.message());)
		exit(0);
	}
	DEBUG_LOG(Log::log("UDP socket configured");)
}

void RTDS::mConfigCCMserver()
{
	DEBUG_LOG(Log::log("CCM server configuring...");)
		try {
		mCCMcontext.set_options(asio::ssl::context::default_workarounds
			| asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
		DEBUG_LOG(Log::log("CCM context options configured");)
		mCCMcontext.use_certificate_file("server_cert.pem", asio::ssl::context::pem);
		DEBUG_LOG(Log::log("CCM SSL certificate loaded");)
		mCCMcontext.use_private_key_file("server_cert.pem", asio::ssl::context::pem);
		DEBUG_LOG(Log::log("CCM SSL private key loaded");)
		mCCMcontext.use_tmp_dh_file("dh2048.pem");
		DEBUG_LOG(Log::log("CCM SSL DH files loaded");)
		mCCMacceptor.open(mCCMep.protocol());
		DEBUG_LOG(Log::log("CCM acceptor open");)
		mCCMacceptor.bind(mCCMep);
		DEBUG_LOG(Log::log("CCM acceptor bound to endpoint");)
		mCCMacceptor.listen(asio::socket_base::max_listen_connections);
		DEBUG_LOG(Log::log("CCM acceptor started listening");)
	}
	catch (const asio::error_code ec)
	{
		LOG(Log::log("Failed to configure CCM server - ", ec.message());)
		exit(0);
	}
	DEBUG_LOG(Log::log("CCM server configured");)
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
		catch (const std::runtime_error& ec)
		{
			LOG(Log::log("Cannot spawn IO Thread - ", ec.what());)
			exit(0);
		}
	}
}

void RTDS::mTCPacceptRoutine()
{
	mThreadCount++;
	asio::socket_base::keep_alive keepAlive(true);
	asio::socket_base::enable_connection_aborted connAbortSignal(true);

	while (mServerRunning)
	{
		asio::ip::tcp::socket* peerSocket = nullptr;
		bool peerIsGood = true;
		try
		{
			peerSocket = new asio::ip::tcp::socket(mIOcontext);
			DEBUG_LOG(Log::log("TCP socket created");)
			mTCPacceptor.accept(*peerSocket);
			DEBUG_LOG(Log::log("TCP socket accepted connection");)
			peerSocket->set_option(keepAlive);
			DEBUG_LOG(Log::log("CCM socket option keepAlive set");)
			peerSocket->set_option(connAbortSignal);
			DEBUG_LOG(Log::log("CCM socket option connAbortSignal set");)
			new TCPpeer(peerSocket);
			DEBUG_LOG(Log::log("CCM peer created");)
		}
		catch (const std::runtime_error& ec)
		{
			DEBUG_LOG(Log::log("Cannot allocate CCM peer/socket - ", ec.what());)
			peerIsGood = false;
		}
		catch (const asio::error_code& ec)
		{
			DEBUG_LOG(Log::log("Failed to configure CCM peer - ", ec.message());)
			peerIsGood = false;
		}

		if (!peerIsGood && peerSocket != nullptr)
			delete peerSocket;
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
		{	if (mServerRunning) { DEBUG_LOG(Log::log("UDP receive failed - ", ec.message());)	}}
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

void RTDS::mCCMacceptRoutine()
{
	mThreadCount++;
	while (mServerRunning)
	{


		auto peerSocket = new (std::nothrow) SSLsocket(mIOcontext, mCCMcontext);
		if (peerSocket == nullptr)
		{
			LOG(Log::log("CCM Peer socket bad allocation");)
			std::this_thread::sleep_for(std::chrono::seconds(1));
			continue;
		}

		asio::error_code ec;
		mCCMacceptor.accept(peerSocket->lowest_layer(), ec);
		if (ec)
		{
			if (mServerRunning) { DEBUG_LOG(Log::log("CCM Acceptor failed - ", ec.message());)	}
			delete peerSocket;
		}
		else
		{
			peerSocket->handshake(asio::ssl::stream_base::server, ec);
			if (ec)
			{	
				DEBUG_LOG(Log::log("SSL handshake failed - ", ec.message());)
				delete peerSocket;
			}
			else
			{
				asio::socket_base::keep_alive keepAlive(true);
				asio::socket_base::enable_connection_aborted connAbortSignal(true);
				try {
					peerSocket->lowest_layer().set_option(keepAlive);
					peerSocket->lowest_layer().set_option(connAbortSignal);
				}
				catch (asio::error_code& ec)
				{	DEBUG_LOG(Log::log("CCM set_option(keepAlive | connectionAbort) failed - ", ec.message());)	}

				auto peer = new (std::nothrow) SSLccm(peerSocket);
				if (peer == nullptr)
				{
					LOG(Log::log("Peer memmory bad allocation");)
					delete peerSocket;
				}
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

void RTDS::mStopCCMserver()
{
	asio::error_code ec;
	mCCMacceptor.cancel(ec);
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

	mCCMacceptor.close(ec);
	if (ec)
	{	DEBUG_LOG(Log::log("SSL socket cannot close - ", ec.message());)	}
}
