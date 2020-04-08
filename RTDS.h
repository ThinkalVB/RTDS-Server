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
	unsigned short threadCount;

	bool TCPserverRunning = false;
	bool TCPaccepting = false;

	void _ioThreadJob();
	void _acceptThreadJob();

public:

	RTDS(int = 200);
	bool startTCPserver();
	bool addThread(int threadCount = 1);
	bool startTCPaccepting();
	bool stopTCPserver();
	~RTDS();
};

