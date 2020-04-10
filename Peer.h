#pragma once
#include <boost/asio.hpp>
#include <boost/date_time.hpp>
#include <list>

using namespace boost;
constexpr unsigned short RTDS_BUFF_SIZE = 300;

class Peer
{
	static std::list<Peer*> peerPtrContainer;
	static std::mutex peerContainerLock;

	asio::ip::tcp::socket* peerSocket;
	posix_time::ptime startTime;
	asio::ip::tcp::endpoint remoteEp;
	std::string UID;
	uint8_t sourcePair[18];

	char dataBuffer[RTDS_BUFF_SIZE];
	std::string writeBuffer;

	void _peerReceiveData();
	void _sendPeerData();

	void _processData(const boost::system::error_code&, std::size_t);
	void _sendData(const boost::system::error_code&, std::size_t);
public:
	Peer(asio::ip::tcp::socket*);
	~Peer();

	friend class RTDS;
	friend class CmdInterpreter;
};