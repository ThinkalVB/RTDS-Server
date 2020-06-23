#include "rtds.h"
#include <thread>
#include "rtds_settings.h"
#include "cmd_processor.h"
#include "peer.h"
#include "log.h"

#ifdef RTDS_DUAL_STACK
RTDS::RTDS(const unsigned short portNumber, short threadCount) : m_tcpEp(asio::ip::address_v6::any(), portNumber), m_udpEp(asio::ip::address_v6::any(), portNumber), 
m_udpSock(m_ioContext), m_tcpAcceptor(m_ioContext), m_worker(m_ioContext)
#else 
RTDS::RTDS(const unsigned short portNumber, short threadCount) : m_tcpEp(asio::ip::address_v4::any(), portNumber), m_udpEp(asio::ip::address_v4::any(), portNumber),
m_udpSock(m_ioContext), m_tcpAcceptor(m_ioContext), m_worker(m_ioContext)
#endif
{
	START_LOG
	DEBUG_LOG(Log::log("............... RTDS Log ..............");)
	DEBUG_LOG(Log::log("RTDS Port : ", portNumber);)
	m_threadCount = 0;
	m_addthread(threadCount - 2);
	m_configTCPserver();
	m_configUDPserver();
}

RTDS::~RTDS()
{
	if (m_serverRunning)
		stopServer();
	
	m_closeSockets();
	m_ioContext.stop();
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} while (!m_ioContext.stopped() && m_threadCount == 0);

	DEBUG_LOG(Log::log("RTDS Exiting [Logging stopped]");)
	STOP_LOG
}

void RTDS::startServer()
{
	m_ioContext.restart();
	m_serverRunning = true;
	try {
		std::thread ioThreadLR(&RTDS::m_udpListenRoutine, this);
		std::thread ioThreadAR(&RTDS::m_tcpAcceptRoutine, this);
		ioThreadLR.detach();
		ioThreadAR.detach();
		DEBUG_LOG(Log::log("New thread to UDP Listen && TCP Accept routines");)
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
	m_serverRunning = false;
	m_stopTCPserver();
	m_stopUDPserver();
}

bool RTDS::isActive()
{
	return m_serverRunning;
}

void RTDS::printStatus()
{
	std::cout << "Active Threads : " << m_threadCount << "\t\t";
	std::cout << "RTDS Port      : " << m_tcpEp.port() << std::endl;

	if (m_serverRunning)
		std::cout << "RTDS Running   : OK " << "\t\t";
	else
		std::cout << "RTDS Running   : OK " << "\t\t";
	std::cout << "Connections    : " << Peer::peerCount() << std::endl;

	std::cout << "Warning        : " << WARNINGS << "\t\t";
	std::cout << "Memmory Error  : " << MEMMORY_ERR << std::endl;

	std::cout << "Socket Error   : " << SOCKET_ERR << "\t\t";
	std::cout << "IO Error       : " << IO_ERR << std::endl;
	std::cout << "Code Error     : " << CODE_ERR << std::endl;
}

void RTDS::m_configTCPserver()
{
	asio::error_code ec;
	m_tcpAcceptor.open(m_tcpEp.protocol(), ec);
	if (ec)
	{
		LOG(Log::log("Failed to open TCP acceptor - ", ec.message());)
		REGISTER_SOCKET_ERR
		exit(0);
	}
	DEBUG_LOG(Log::log("TCP acceptor open");)

	m_tcpAcceptor.bind(m_tcpEp, ec);
	if (ec)
	{
		m_tcpAcceptor.close();
		LOG(Log::log("Failed to bind TCP acceptor - ", ec.message());)
		REGISTER_SOCKET_ERR

	}
	DEBUG_LOG(Log::log("TCP acceptor binded to endpoint");)

	m_tcpAcceptor.listen(asio::socket_base::max_listen_connections, ec);
	if (ec)
	{
		m_tcpAcceptor.close();
		LOG(Log::log("TCP acceptor cannot listen to port - ", ec.message());)
		REGISTER_SOCKET_ERR
		exit(0);
	}
	DEBUG_LOG(Log::log("TCP acceptor listening to port");)
}

void RTDS::m_configUDPserver()
{
	asio::error_code ec;
	m_udpSock.open(m_udpEp.protocol(), ec);
	if (ec)
	{
		LOG(Log::log("Failed to open UDP acceptor - ", ec.message());)
		REGISTER_SOCKET_ERR
		exit(0);
	}
	DEBUG_LOG(Log::log("UDP sock open");)

	m_udpSock.bind(m_udpEp, ec);
	if (ec)
	{
		m_udpSock.close();
		LOG(Log::log("Failed to bind UDP socket - ", ec.message());)
		REGISTER_SOCKET_ERR
		exit(0);
	}
	DEBUG_LOG(Log::log("UDP socket binded to endpoint");)
}

void RTDS::m_ioThreadJob()
{
	asio::error_code ec;
	m_threadCount++;
	m_ioContext.run(ec);
	if (ec)
	{
		LOG(Log::log("ioContext.run() failed - ", ec.message());)
		REGISTER_IO_ERR
	}

	m_threadCount--;
	DEBUG_LOG(Log::log("ioContext thread exiting");)
}

void RTDS::m_addthread(int threadCount)
{
	for (int i = 0; i < threadCount; i++)
	{
		try {
			std::thread ioThread(&RTDS::m_ioThreadJob, this);
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

void RTDS::m_tcpAcceptRoutine()
{
	m_threadCount++;
	while (m_serverRunning)
	{
		asio::ip::tcp::socket* peerSocket = nullptr;
		try
		{	peerSocket = new asio::ip::tcp::socket(m_ioContext);	}
		catch (std::bad_alloc) {
			LOG(Log::log("Peer socket bad allocation");)
			REGISTER_MEMMORY_ERR
		}

		asio::error_code ec;
		m_tcpAcceptor.accept(*peerSocket, ec);
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

			try 
			{	new Peer(peerSocket);	}
			catch (std::bad_alloc) {
				LOG(Log::log("Peer memmory bad allocation");)
				REGISTER_MEMMORY_ERR
				peerSocket->close();
				delete peerSocket;
			}
		}
	}
	m_threadCount--;
}

void RTDS::m_udpListenRoutine()
{
	m_threadCount++;
	while (m_serverRunning)
	{
		asio::ip::udp::endpoint udpEp;
		ReceiveBuffer udpBuffer;
		asio::error_code ec;

		auto size = m_udpSock.receive_from(asio::buffer(udpBuffer.data(), RTDS_BUFF_SIZE), udpEp, 0, ec);
		if (ec)
		{	
			DEBUG_LOG(Log::log("UDP receive failed - ", ec.message());)	
			REGISTER_SOCKET_ERR
		}
		else
		{
			udpBuffer[size] = '\0';
			auto response = CmdProcessor::processCommand(udpBuffer, udpEp);
			m_udpSock.send_to(asio::buffer(response.data(), response.size()), udpEp);
		}
	}
	m_threadCount--;
}

void RTDS::m_stopUDPserver()
{
	asio::error_code ec;
	m_udpSock.cancel(ec);
	if (ec)
	{	
		DEBUG_LOG(Log::log("UDP socket cannot cancel - ", ec.message());)	
		REGISTER_SOCKET_ERR
	}
}

void RTDS::m_stopTCPserver()
{
	asio::error_code ec;
	m_tcpAcceptor.cancel(ec);
	if (ec)
	{	
		DEBUG_LOG(Log::log("TCP acceptor cannot cancel - ", ec.message());)	
		REGISTER_SOCKET_ERR
	}
}

void RTDS::m_closeSockets()
{
	asio::error_code ec;
	m_udpSock.close(ec);
	if (ec)
	{	
		DEBUG_LOG(Log::log("UDP socket cannot close - ", ec.message());)	
		REGISTER_SOCKET_ERR
	}

	m_tcpAcceptor.close(ec);
	{	
		DEBUG_LOG(Log::log("UDP socket cannot close - ", ec.message());)
		REGISTER_SOCKET_ERR
	}
}
