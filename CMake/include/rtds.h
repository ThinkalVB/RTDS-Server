#ifndef RTDS_H
#define RTDS_H

#include <asio.hpp>
#include "common.h"

class RTDS
{
	asio::io_context m_ioContext;				// ioContext controls all the async functions related to ASIO
	asio::io_context::work m_worker;			// Worker object to prevent ioContext.run() from exiting when without async jobs
	asio::ip::tcp::endpoint m_tcpEp;			// TCP endpoint that describe the IPaddr ,Port and Protocol for the acceptor socket
	asio::ip::tcp::acceptor m_tcpAcceptor;		// TCP acceptor socket that accept incoming tcp connections

	asio::ip::udp::endpoint m_udpEp;			// UDP endpoint that describe the IPaddr ,Port and Protocol for the socket
	asio::ip::udp::socket m_udpSock;			// UDP socket that accept packets

	int m_threadCount;							// Keep account of number of threads running ioContex.run()
	std::atomic_bool m_serverRunning;			// True if the server is running

	void m_configTCPserver();
	void m_configUDPserver();
	
	void m_ioThreadJob();
	void m_addthread(const int);

	void m_tcpAcceptRoutine();
	void m_udpListenRoutine();


	void m_stopUDPserver();
	void m_stopTCPserver();
	void m_closeSockets();

public:
/*******************************************************************************************
* @brief Create the server object with it's own ioContext and worker class object
*
* @param[in]		The default port is 389
* @param[in]		Number of threads to run the RTDS with [2-28]
*
* @details
* All the STL containers associated with this class are static in nature.
* Add #define RTDS_DUAL_STACK in RTDS.h to compile the RTDS in IPv6 dual stack mode.
* Start the logging system.
********************************************************************************************/
	RTDS(const unsigned short, short);
	~RTDS();

	void startServer();
	void stopServer();
	bool isActive();
	void printStatus();
};

#endif