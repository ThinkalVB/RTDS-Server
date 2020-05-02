#include "peer.h"
#include <boost/bind.hpp>
#include "cmd_interpreter.h"
#include "directory.h"
#include "log.h"

short Peer::peerCount = 0;
int Peer::notificationCount = 0;
std::list<Notification> Peer::notificationList;

std::vector<Peer*> Peer::mirroringGroup;
std::mutex Peer::MListLock;
std::mutex Peer::NListLock;

Peer::Peer(asio::ip::tcp::socket* socketPtr)
{
	peerSocket = socketPtr;
	writeBuffer.reserve(RTDS_BUFF_SIZE);
	remoteEp = socketPtr->remote_endpoint();

	if (remoteEp.address().is_v4())
		peerEntry.EvB = Directory::makeEntry(remoteEp.address().to_v4(), remoteEp.port());
	else
		peerEntry.EvB = Directory::makeEntry(remoteEp.address().to_v6(), remoteEp.port());

	peerEntry.EvB->attachToPeer();
	_peerReceiveData();
	peerCount++;
}

void Peer::_peerReceiveData()
{
	peerSocket->async_receive(asio::buffer(dataBuffer.data(), RTDS_BUFF_SIZE), 0, bind(&Peer::_processData,
		this, asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void Peer::_processData(const boost::system::error_code& ec, std::size_t size)
{
	if (ec != system::errc::success)
	{
		#ifdef PRINT_LOG
		Log::log("TCP Socket _processData() failed", peerSocket, ec);
		#endif
		terminatePeer();
	}
	else
	{
		if (CmdInterpreter::makeCmdElement(dataBuffer, commandElement, size))
			CmdInterpreter::processCommand(*this);
		else
		{
			writeBuffer = CmdInterpreter::RESP[(short)Response::BAD_COMMAND];
			writeBuffer += '\x1e';				//!< Record separator
			sendPeerData();
		}
	}
}

void Peer::_sendData(const boost::system::error_code& ec, std::size_t size)
{
	if (ec != system::errc::success)
	{
		#ifdef PRINT_LOG
		Log::log("TCP Socket _sendData() failed", peerSocket, ec);
		#endif
		terminatePeer();
	}
	else
	{
		writeBuffer.clear();
		_peerReceiveData();
	}
}

void Peer::_sendNotification(const boost::system::error_code& ec, std::size_t size)
{
	if (ec != system::errc::success)
	{
		#ifdef PRINT_LOG
		Log::log("TCP Socket _sendNotification() failed", peerSocket, ec);
		#endif
		terminatePeer();
	}
}

void Peer::sendPeerData()
{
	peerSocket->async_send(asio::buffer(writeBuffer.data(), writeBuffer.size()), bind(&Peer::_sendData,
		this, asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void Peer::sendNotification(const std::string& noteString)
{
	std::lock_guard<std::mutex> lock(MListLock);
	NListLock.lock();
	notificationCount++;
	auto note = notificationList.emplace_back(noteString, notificationCount);
	if (notificationList.size() > MAX_NOTE_SIZE)
		notificationList.pop_front();
	NListLock.unlock();

	for (int i = 0; i < mirroringGroup.size(); i++)
	{
		if (mirroringGroup[i] != this)
		{
			mirroringGroup[i]->peerSocket->async_send(asio::buffer(note.noteString.data(), note.noteString.size()),
				bind(&Peer::_sendNotification, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
		}
	}
}


void Peer::terminatePeer()
{
	peerSocket->shutdown(asio::ip::tcp::socket::shutdown_both);
	delete this;
}

short Peer::getPeerCount()
{
	return peerCount;
}

void Peer::addToMirroringGroup()
{
	std::lock_guard<std::mutex> lock(MListLock);
	if (!isMirroring)
	{
		isMirroring = true;
		mirroringGroup.push_back(this);
	}
}

void Peer::removeFromMirroringGroup()
{
	std::lock_guard<std::mutex> lock(MListLock);
	if (isMirroring)
	{
		isMirroring = false;
		auto itr = std::find(mirroringGroup.begin(), mirroringGroup.end(), this);
		std::iter_swap(itr, mirroringGroup.end() - 1);
		mirroringGroup.pop_back();
	}
}

BaseEntry* Peer::entry()
{
	return peerEntry.EvB;
}

std::string& Peer::Buffer()
{
	return writeBuffer;
}

CommandElement& Peer::cmdElement()
{
	return commandElement;
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

	peerEntry.EvB->detachFromPeer();
	removeFromMirroringGroup();
	peerCount--;
	delete peerSocket;
}