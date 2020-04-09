#pragma once
#include <boost/asio.hpp>

constexpr unsigned short RTDS_PORT = 389;
using namespace boost;

class RTDS
{
	asio::io_context ioContext;
	asio::io_context::work worker;
	asio::ip::tcp::endpoint tcpEp;
	asio::ip::tcp::acceptor tcpAcceptor;

	unsigned short activeThreadCount;
	std::atomic<bool> tcpServerRunning;
	std::atomic<bool> keepAccepting;

	void _ioThreadJob();
	void _peerAcceptRoutine();
	void _stopIoContext();
	void _stopTCPacceptor();

public:

	RTDS(unsigned short = RTDS_PORT);
	bool startTCPserver();
	bool addThread(int threadCount = 1);
	void stopTCPserver();
	~RTDS();
};

