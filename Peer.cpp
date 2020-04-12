#include "Peer.h"
#include <boost/bind.hpp>
#include <cppcodec/base64_rfc4648.hpp>
#include <string_view>
#include "CmdInterpreter.h"
#include "Log.h"

std::list<Peer*> Peer::peerPtrContainer;
std::mutex Peer::peerContainerLock;

Peer::Peer(asio::ip::tcp::socket* socketPtr)
{
	peerSocket = socketPtr;
	writeBuffer.reserve(RTDS_BUFF_SIZE);

	remoteEp = socketPtr->remote_endpoint();
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
		#ifdef PRINT_LOG
		Log::log("TCP Socket _processData() failed", peerSocket);
		#endif

		std::lock_guard<std::mutex> lock(Peer::peerContainerLock);
		peerPtrContainer.remove(this);
		delete this;
	}
	else
	{
		if (dataBuffer[size - 1] == ';')
		{
			dataBuffer[size - 1] = '\0';
			CmdInterpreter::processCommand(*this);
		}
		else
		{
			writeBuffer = "bad_command";
			_sendPeerData();
		}
	}
}

void Peer::_sendPeerData()
{
	peerSocket->async_send(asio::buffer(writeBuffer.data(), writeBuffer.size()), bind(&Peer::_sendData,
		this, asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void Peer::_sendData(const boost::system::error_code& ec, std::size_t size)
{
	if (ec != system::errc::success)
	{
		#ifdef PRINT_LOG
		Log::log("TCP Socket _sendData() failed", peerSocket);
		#endif

		std::lock_guard<std::mutex> lock(Peer::peerContainerLock);
		peerPtrContainer.remove(this);
		delete this;
	}
	else
	{
		writeBuffer.clear();
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