#include "peer.h"
#include <boost/bind.hpp>
#include "directory.h"
#include "cmd_interpreter.h"
#include "log.h"

unsigned short Peer::peerCount = 0;

Peer::Peer(asio::ip::tcp::socket* socketPtr)
{
	peerSocket = socketPtr;
	writeBuffer.reserve(RTDS_BUFF_SIZE);
	remoteEp = socketPtr->remote_endpoint();

	if (remoteEp.address().is_v4())
	{
		peerEntry.Ev4 = Directory::makeEntry(remoteEp.address().to_v4(), remoteEp.port());
		peerEntry.Ev4->attachToPeer();
	}
	else
	{
		peerEntry.Ev6 = Directory::makeEntry(remoteEp.address().to_v6(), remoteEp.port());
		peerEntry.Ev6->attachToPeer();
	}
	_peerReceiveData();
	peerCount++;
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
		Log::log("TCP Socket _processData() failed", peerSocket, ec);
		#endif
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
			writeBuffer = Response::BAD_COMMAND;
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
		Log::log("TCP Socket _sendData() failed", peerSocket, ec);
		#endif
		delete this;
	}
	else
	{
		writeBuffer.clear();
		_peerReceiveData();
	}
}

unsigned short Peer::getPeerCount()
{
	return peerCount;
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
		Log::log("TCP Socket cannot close", peerSocket, ec);
		#endif
	}

	if (remoteEp.address().is_v4())
		peerEntry.Ev4->detachFromPeer();
	else
		peerEntry.Ev6->detachFromPeer();
	peerCount--;
	delete peerSocket;
}