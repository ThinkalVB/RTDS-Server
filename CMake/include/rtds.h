#ifndef RTDS_H
#define RTDS_H

#include <asio/ip/tcp.hpp>
#include <asio/ip/udp.hpp>
#include <asio/ssl.hpp>

class RTDS
{
	asio::io_context mIOcontext;				// ioContext controls all the async functions related to ASIO
	asio::io_context::work mIOworker;			// Worker object to prevent ioContext.run() from exiting when without async jobs
	
	asio::ip::tcp::endpoint mTCPep;				// TCP endpoint that describe the IPaddr ,Port and Protocol for the acceptor socket
	asio::ip::tcp::acceptor mTCPacceptor;		// TCP acceptor socket that accept incoming tcp connections

	asio::ip::udp::endpoint mUDPep;				// UDP endpoint that describe the IPaddr ,Port and Protocol for the socket
	asio::ip::udp::socket mUDPsock;				// UDP socket that accept packets
	
	asio::ssl::context mCCMcontext;				// CCM context
	asio::ip::tcp::endpoint mCCMep;				// SSL endpoint that describe the IPaddr ,Port and Protocol for the acceptor socket
	asio::ip::tcp::acceptor mCCMacceptor;		// SSL acceptor socket that accept incoming tcp connections	

	//asio::ssl::context mSSLcontext;			// SSL context
	//asio::ip::tcp::endpoint mSSLep;			// SSL endpoint that describe the IPaddr ,Port and Protocol for the acceptor socket
	//asio::ip::tcp::acceptor mSSLacceptor;		// SSL acceptor socket that accept incoming tcp connections	

	int mThreadCount;							// Keep account of number of threads running ioContex.run()
	std::atomic_bool mServerRunning;			// True if the server is running

	void mConfigTCPserver();
	void mConfigUDPserver();
	void mConfigCCMserver();
	
	void mIOthreadJob();
	void mAddthread(const int);

	void mTCPacceptRoutine();
	void mUDPlistenRoutine();
	void mCCMacceptRoutine();

	void mStartServer();
	void mStopServer();

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
	RTDS(const unsigned short, const unsigned short, short);
	~RTDS();
};

#endif