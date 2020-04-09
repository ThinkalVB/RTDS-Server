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

	asio::ip::tcp::socket* peerSocket = nullptr;
	posix_time::ptime startTime;
	char dataBuffer[RTDS_BUFF_SIZE];

	void _peerReceiveData();
	void _processData(const boost::system::error_code&, std::size_t);
public:
	Peer(asio::ip::tcp::socket*);
	~Peer();

	friend class RTDS;
};