#pragma once
#include <boost/asio.hpp>
#include "SPentry.h"
#include <list>

using namespace boost;
constexpr unsigned short RTDS_BUFF_SIZE = 300;

class Peer
{
	static std::list<Peer*> peerPtrContainer;
	static std::mutex peerContainerLock;

	asio::ip::tcp::socket* peerSocket;
	asio::ip::tcp::endpoint remoteEp;
	SPentry peerEntry;

	char dataBuffer[RTDS_BUFF_SIZE];
	std::string writeBuffer;

	void _peerReceiveData();
	void _sendPeerData();

	void _processData(const boost::system::error_code&, std::size_t);
	void _sendData(const boost::system::error_code&, std::size_t);
public:
	Peer(asio::ip::tcp::socket*);
	static void _removeAllPeers();
	~Peer();

	friend class CmdInterpreter;
};