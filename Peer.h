#pragma once
#include <boost/asio.hpp>
#include <boost/date_time.hpp>
#include <string>

using namespace boost;

class Peer
{
	asio::ip::tcp::socket* peerSocket = nullptr;
	posix_time::ptime startTime;
	char dataBuffer[250];
public:
	Peer(asio::ip::tcp::socket*);
	void processData(const boost::system::error_code&, std::size_t);
	~Peer();
};

