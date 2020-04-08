#pragma once
#include <boost/asio.hpp>
#include <vector>
#include "Peer.h"
using namespace boost;

constexpr unsigned short RTDS_PORT = 389;

class RTDS
{
	asio::io_context ioContext;
	asio::io_context::work worker;
	asio::ip::tcp::endpoint tcpEp;
	asio::ip::tcp::acceptor tcpAcceptor;
	std::vector<Peer*> peerHandler;
	unsigned short activeThreadCount;

	bool _tcpServerRunning = false;
	bool _keepAccepting = true;

	void _ioThreadJob();
	void _peerAcceptRoutine();

public:

	RTDS(unsigned short = RTDS_PORT, int = 100);
	bool startTCPserver();
	bool addThread(int threadCount = 1);
	bool stopTCPserver();
	~RTDS();
};

