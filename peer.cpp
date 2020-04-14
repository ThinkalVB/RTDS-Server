#include "peer.h"
#include <boost/bind.hpp>
#include "directory.h"
#include "cmd_interpreter.h"
#include "log.h"

std::list<Peer*> Peer::peerPtrContainer;
std::mutex Peer::peerContainerLock;

Peer::Peer(asio::ip::tcp::socket* socketPtr)
{
	peerSocket = socketPtr;
	writeBuffer.reserve(RTDS_BUFF_SIZE);
	remoteEp = socketPtr->remote_endpoint();

	std::lock_guard<std::mutex> lock(peerContainerLock);
	Peer::peerPtrContainer.push_back(this);

	if (remoteEp.address().is_v4())
	{
		peerEntry.Ev4 = Directory::makeV4Entry(remoteEp.address().to_v4(), remoteEp.port());
		peerEntry.Ev4->attachToPeer();
	}
	else
	{
		peerEntry.Ev6 = Directory::makeV6Entry(remoteEp.address().to_v6(), remoteEp.port());
		peerEntry.Ev6->attachToPeer();
	}
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

void Peer::removeAllPeers()
{
	for (auto lstItr = peerPtrContainer.rbegin(); lstItr != peerPtrContainer.rend(); lstItr++)
		delete* lstItr;
	Peer::peerPtrContainer.clear();
}

void Peer::_sendData(const boost::system::error_code& ec, std::size_t size)
{
	if (ec != system::errc::success)
	{
		#ifdef PRINT_LOG
		Log::log("TCP Socket _sendData() failed", peerSocket);
		#endif
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

	if (remoteEp.address().is_v4())
		peerEntry.Ev4->detachFromPeer();
	else
		peerEntry.Ev6->detachFromPeer();

	std::lock_guard<std::mutex> lock(peerContainerLock);
	peerPtrContainer.remove(this);
	delete peerSocket;
}