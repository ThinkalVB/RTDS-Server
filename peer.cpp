#include "peer.h"
#include <boost/bind.hpp>
#include "directory.h"
#include "cmd_interpreter.h"
#include "log.h"

short Peer::peerCount = 0;
DLLController<Peer> Peer::dllController;

Peer::Peer(asio::ip::tcp::socket* socketPtr)
{
	peerSocket = socketPtr;
	writeBuffer.reserve(RTDS_BUFF_SIZE);
	remoteEp = socketPtr->remote_endpoint();

	if (remoteEp.address().is_v4())
		peerEntry = Directory::makeEntry(remoteEp.address().to_v4(), remoteEp.port());
	else
		peerEntry = Directory::makeEntry(remoteEp.address().to_v6(), remoteEp.port());

	peerEntry->attachToPeer();
	lastNoteNumber = Notification::lastNoteNumber();
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

void Peer::_notifyAll(const Note& note)
{
	if (Peer::dllController.begin() == nullptr)
		return;
	else
	{
		auto peerPtr = Peer::dllController.begin();
		while (peerPtr != nullptr)
		{
			if (peerPtr != this)
			{
				peerPtr->peerSocket->async_send(asio::buffer(note.noteString.data(), note.noteString.size()),
					bind(&Peer::_sendNotification, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
			}
			peerPtr = peerPtr->dllNode.next();
		}
	}
}

void Peer::sendPeerData()
{
	peerSocket->async_send(asio::buffer(writeBuffer.data(), writeBuffer.size()), bind(&Peer::_sendData,
		this, asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void Peer::sendNotification(const std::string& noteString)
{
	auto note = Notification::newNotification(noteString);
	_notifyAll(note);
}

void Peer::syncUpdate()
{
	Notification::createNoteRecord(writeBuffer, lastNoteNumber);
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
	if (!isMirroring)
	{
		isMirroring = true;
		dllNode.addToDLL(this);
	}
}

void Peer::removeFromMirroringGroup()
{
	if (isMirroring)
	{
		isMirroring = false;
		dllNode.removeFromDLL(this);
		lastNoteNumber = Notification::lastNoteNumber();
	}
}

Entry* Peer::entry()
{
	return peerEntry;
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

	peerEntry->detachFromPeer();
	removeFromMirroringGroup();
	peerCount--;
	delete peerSocket;
}