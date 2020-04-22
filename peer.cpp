#include "peer.h"
#include <boost/bind.hpp>
#include "cmd_interpreter.h"
#include "directory.h"
#include "log.h"

short Peer::peerCount = 0;
std::vector<Peer*> Peer::mirroringGroup;
std::mutex Peer::mirroringListLock;

Peer::Peer(asio::ip::tcp::socket* socketPtr)
{
	peerSocket = socketPtr;
	writeBuffer.reserve(RTDS_BUFF_SIZE);
	remoteEp = socketPtr->remote_endpoint();

	if (remoteEp.address().is_v4())
		peerEntry.Ev = Directory::makeEntry(remoteEp.address().to_v4(), remoteEp.port());
	else
		peerEntry.Ev = Directory::makeEntry(remoteEp.address().to_v6(), remoteEp.port());

	peerEntry.Ev->attachToPeer();
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
			receivedData = dataBuffer;
			CmdInterpreter::processCommand(*this);
		}
		else
		{
			writeBuffer = CmdInterpreter::RESP[(short)Response::BAD_COMMAND];
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

short Peer::getPeerCount()
{
	return peerCount;
}

void Peer::addToMirroringGroup()
{
	std::lock_guard<std::mutex> lock(mirroringListLock);
	if (!isMirroring)
	{
		isMirroring = true;
		mirroringGroup.push_back(this);
	}
}

void Peer::removeFromMirroringGroup()
{
	std::lock_guard<std::mutex> lock(mirroringListLock);
	if (isMirroring)
	{
		isMirroring = false;
		auto itr = std::find(mirroringGroup.begin(), mirroringGroup.end(), this);
		std::iter_swap(itr, mirroringGroup.end() - 1);
		mirroringGroup.pop_back();
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
		Log::log("TCP Socket cannot close", peerSocket, ec);
		#endif
	}

	peerEntry.Ev4->detachFromPeer();
	removeFromMirroringGroup();
	peerCount--;
	delete peerSocket;
}