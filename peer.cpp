#include "peer.h"
#include <boost/bind.hpp>
#include "cmd_interpreter.h"
#include "log.h"

short Peer::_peerCount = 0;
std::vector<Peer*> Peer::_mirrorGroup;
std::mutex Peer::_mgAccessLock;

Peer::Peer(asio::ip::tcp::socket* socketPtr) :_remoteEp(socketPtr->remote_endpoint()), spAddress(_remoteEp)
{
	_peerSocket = socketPtr;
	writeBuffer.reserve(RTDS_BUFF_SIZE);
	_peerIsActive = true;

	_peerReceiveData();
	_peerCount++;
}

void Peer::_peerReceiveData()
{
	_peerSocket->async_receive(asio::buffer(_dataBuffer.data(), RTDS_BUFF_SIZE), 0, bind(&Peer::_processData,
		this, asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void Peer::_processData(const boost::system::error_code& ec, std::size_t size)
{
	if (ec != system::errc::success)
	{
		#ifdef PRINT_LOG
		Log::log("TCP Socket _processData() failed", _peerSocket, ec);
		#endif
		terminatePeer();
	}
	else
	{
		std::lock_guard<std::mutex> lock(_resourceMtx);
		if (CmdInterpreter::makeCmdElement(_dataBuffer, cmdElement, size))
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
		Log::log("TCP Socket _sendData() failed", _peerSocket, ec);
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
		Log::log("TCP Socket _sendNotification() failed", _peerSocket, ec);
		#endif
		terminatePeer();
	}
}

void Peer::sendPeerData()
{
	if (_peerIsActive)
	_peerSocket->async_send(asio::buffer(writeBuffer.data(), writeBuffer.size()), bind(&Peer::_sendData,
		this, asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void Peer::terminatePeer()
{
	_peerIsActive = false;
	_peerSocket->shutdown(asio::ip::tcp::socket::shutdown_both);
	delete this;
}

short Peer::getPeerCount()
{
	return _peerCount;
}

void Peer::addToMG(const MutableData& policy)
{
	if (!_isMirroring)
	{
		std::lock_guard<std::mutex> lock(_mgAccessLock);
		_isMirroring = true;
		_mirrorGroup.push_back(this);
		_mirrorPolicy = policy;
	}
	else
		_mirrorPolicy = policy;
}

void Peer::removeFromMG()
{
	if (_isMirroring)
	{
		_isMirroring = false;
		std::lock_guard<std::mutex> lock(_mgAccessLock);
		auto itr = std::find(_mirrorGroup.begin(), _mirrorGroup.end(), this);
		std::iter_swap(itr, _mirrorGroup.end() - 1);
		_mirrorGroup.pop_back();
	}
}

void Peer::sendNoteToMG(const Note& note)
{
	std::lock_guard<std::mutex> lock(_mgAccessLock);
	for (auto peerItr = _mirrorGroup.begin(); peerItr != _mirrorGroup.end(); ++peerItr)
	{
		if ((*peerItr)->_peerIsActive)
		(*peerItr)->_peerSocket->async_send(asio::buffer(note.noteString.data(), note.noteString.size()),
			bind(&Peer::_sendNotification, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
	}
}

Peer::~Peer()
{
	#ifdef PRINT_LOG
	Log::log("TCP Socket closing", _peerSocket);
	#endif
	system::error_code ec;
	_peerSocket->close(ec);
	if (ec != system::errc::success)
	{
		#ifdef PRINT_LOG
		Log::log("TCP Socket cannot close", _peerSocket, ec);
		#endif
	}

	removeFromMG();
	_peerCount--;
	std::lock_guard<std::mutex> lock(_resourceMtx);
	delete _peerSocket;
}