#include "Peer.h"
#include <boost/bind.hpp>
#include "Log.h"

Peer::Peer(asio::ip::tcp::socket* socketPtr)
{
	peerSocket = socketPtr;
	startTime = posix_time::second_clock::local_time();
	peerSocket->async_receive(asio::buffer(dataBuffer,250), 0, bind(&Peer::processData,
		this, asio::placeholders::error,asio::placeholders::bytes_transferred));
}

void Peer::processData(const boost::system::error_code& ec, std::size_t size)
{
#include<iostream>
	std::cout << dataBuffer << std::endl;

}

Peer::~Peer()
{
	system::error_code ec;
	peerSocket->close(ec);
	if (ec != system::errc::success)
	{
		#ifdef PRINT_LOG
		Log::log("TCP Socket cannot close", ec);
		#endif
	}
	delete peerSocket;
}

