#include "rtds.h"
#include <thread>
#include "rtds_settings.h"
#include "peer.h"
#include "log.h"

#ifdef RTDS_DUAL_STACK
RTDS::RTDS(unsigned short portNumber) : m_tcpEp(asio::ip::address_v6::any(), portNumber), m_tcpAcceptor(m_ioContext), m_worker(m_ioContext)
#else 
RTDS::RTDS(unsigned short portNumber) : m_tcpEp(asio::ip::address_v4::any(), portNumber), m_tcpAcceptor(m_ioContext), m_worker(m_ioContext)
#endif
{
	START_LOG
	DEBUG_LOG(Log::log("............... RTDS Log ..............");)
	DEBUG_LOG(Log::log("RTDS Port : ", portNumber);)
}

bool RTDS::startTCPserver(const int threadCount)
{
	DEBUG_LOG(Log::log("TCP server initiating");)
	m_tcpServerRunning = false;
	m_activeThreadCount = 0;
	addThread(threadCount);

	asio::error_code ec;
	m_tcpAcceptor.open(m_tcpEp.protocol(), ec);
	if (ec)
	{
		LOG(Log::log("Failed to open TCP acceptor - ", ec.message());)
		REGISTER_SOCKET_ERR
		return false;
	}
	DEBUG_LOG(Log::log("TCP acceptor open");)

	m_tcpAcceptor.bind(m_tcpEp, ec);
	if (ec)
	{
		m_tcpAcceptor.close();
		LOG(Log::log("Failed to bind TCP acceptor - ", ec.message());)
		REGISTER_SOCKET_ERR
		return false;
	}
	DEBUG_LOG(Log::log("TCP acceptor binded to endpoint");)

	m_tcpAcceptor.listen(asio::socket_base::max_listen_connections, ec);
	if (ec)
	{
		m_tcpAcceptor.close();
		LOG(Log::log("TCP acceptor cannot listen to port - ", ec.message());)
		REGISTER_SOCKET_ERR
		return false;
	}
	DEBUG_LOG(Log::log("TCP acceptor listening to port");)
	startAccepting();
	m_tcpServerRunning = true;

	DEBUG_LOG(Log::log("TCP server started");)
	return true;
}

bool RTDS::addThread(const int threadCount)
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
			return false;
		}
	}
	return true;
}

void RTDS::addThisThread()
{
	DEBUG_LOG(Log::log("Calling thread added to ioContext");)
	m_ioThreadJob();
}

void RTDS::startAccepting()
{
	DEBUG_LOG(Log::log("TCP Accepting starting");)
	m_keepAccepting = true;
	m_peerAcceptRoutine();
}

void RTDS::stopAccepting()
{
	DEBUG_LOG(Log::log("TCP Accepting stopped");)
	m_keepAccepting = false;
	m_tcpAcceptor.cancel();
}

bool RTDS::isAccepting()
{
	return m_keepAccepting;
}

void RTDS::stopTCPserver()
{
	m_stopTCPacceptor();
	m_ioContext.reset();
	DEBUG_LOG(Log::log("TCP server stopped");)

	m_keepAccepting = false;
	m_tcpServerRunning = false;
}

void RTDS::printStatus()
{
	std::cout << "Active Threads : " << m_activeThreadCount << "\t\t";
	std::cout << "RTDS Port      : " << m_tcpEp.port() << std::endl;

	if (m_keepAccepting)
		std::cout << "Accepting      : OK " << "\t\t";
	else
		std::cout << "Accepting      : NO " << "\t\t";
	std::cout << "Connections    : " << Peer::peerCount() << std::endl;

	std::cout << "Warning        : " << WARNINGS << "\t\t";
	std::cout << "Memmory Error  : " << MEMMORY_ERR << std::endl;

	std::cout << "Socket Error   : " << SOCKET_ERR << "\t\t";
	std::cout << "IO Error       : " << IO_ERR << std::endl;
	std::cout << "Code Error     : " << CODE_ERR << std::endl;
}

void RTDS::m_ioThreadJob()
{
	asio::error_code ec;
	m_activeThreadCount++;
	m_ioContext.run(ec);
	if (ec)
	{	
		LOG(Log::log("ioContext.run() failed - ", ec.message());)
		REGISTER_IO_ERR
	}

	m_activeThreadCount--;
	DEBUG_LOG(Log::log("ioContext thread exiting");)
	return;
}

void RTDS::m_peerAcceptRoutine()
{
	auto peerSocket = new asio::ip::tcp::socket(m_ioContext);

	m_tcpAcceptor.async_accept(*peerSocket,
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
			if (m_keepAccepting)
				m_peerAcceptRoutine();
		});
}

void RTDS::m_stopIoContext()
{
	m_ioContext.stop();
	do {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} while (!m_ioContext.stopped() && m_activeThreadCount == 0);
	DEBUG_LOG(Log::log("IO Context stopped");)
}

void RTDS::m_stopTCPacceptor()
{
	asio::error_code ec;
	m_tcpAcceptor.cancel(ec);
	if (ec)
	{	
		LOG(Log::log("TCP acceptor cannot cancel operations - ", ec.message());)	
		REGISTER_SOCKET_ERR
	}

	m_tcpAcceptor.close(ec);
	if (ec)
	{	
		LOG(Log::log("TCP acceptor cannot close - ", ec.message());)	
		REGISTER_SOCKET_ERR
	}
	DEBUG_LOG(Log::log("TCP acceptor stopped");)
}

RTDS::~RTDS()
{
	if (m_tcpServerRunning)
		stopTCPserver();
	m_stopIoContext();

	DEBUG_LOG(Log::log("RTDS Exiting [Logging stopped]");)
	STOP_LOG
}