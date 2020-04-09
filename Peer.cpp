#include "Peer.h"
#include <boost/bind.hpp>
#include <string_view>
#include "Log.h"

std::list<Peer*> Peer::peerPtrContainer;
std::mutex Peer::peerContainerLock;

Peer::Peer(asio::ip::tcp::socket* socketPtr)
{
	peerSocket = socketPtr;
	startTime = posix_time::second_clock::local_time();
	_peerReceiveData();
}

void Peer::_peerReceiveData()
{
	peerSocket->async_receive(asio::buffer(dataBuffer, RTDS_BUFF_SIZE), 0, bind(&Peer::_processData,
		this, asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void Peer::_processData(const boost::system::error_code& ec, std::size_t size)
{
	if (ec != system::errc::success)
	{
		std::lock_guard<std::mutex> lock(Peer::peerContainerLock);
		peerPtrContainer.remove(this);
		delete this;
	}
	else
	{
		auto commandString = std::string_view{ dataBuffer };
		if (dataBuffer[size - 1] == ';')
		{
			dataBuffer[size] = '\0';
			//auto commandEnd = commandString.find(' ');
		}
		_peerReceiveData();
	}
}

Peer::~Peer()
{
	#ifdef PRINT_LOG
	Log::log("TCP Socket closing", peerSocket);
	#endif
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