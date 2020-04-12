#pragma once
#include <boost/asio.hpp>
#include <list>

using namespace boost;
constexpr unsigned short RTDS_BUFF_SIZE = 300;

class Peer
{
	static std::list<Peer*> peerPtrContainer;
	static std::mutex peerContainerLock;

	asio::ip::tcp::socket* peerSocket;
	asio::ip::tcp::endpoint remoteEp;

	char dataBuffer[RTDS_BUFF_SIZE];
	std::string writeBuffer;

	void _peerReceiveData();
	void _sendPeerData();

	void _processData(const boost::system::error_code&, std::size_t);
	void _sendData(const boost::system::error_code&, std::size_t);
	~Peer();
public:
	Peer(asio::ip::tcp::socket*);

	friend class RTDS;
	friend class CmdInterpreter;
};