#include "peer.h"
#include "cmd_processor.h"
#include "bg_controller.h"
#include "log.h"

std::atomic_int Peer::_peerCount = 0;

Peer::Peer(asio::ip::tcp::socket* socketPtr) : _saPair(socketPtr)
{
	_peerSocket = socketPtr;
	_writeBuffer.reserve(RTDS_BUFF_SIZE);

	_peerIsActive = true;
	_bgPtr = nullptr;
	_isInBG = false;

	DEBUG_LOG(Log::ALog(_saPair.toString()," Peer Connected");)
	_peerCount++;
	_peerReceiveData();
}

void Peer::sendMessage(const std::string& bgTag, const Message* message)
{
	std::lock_guard<std::mutex> lock(_resLock);
	if (_peerIsActive && _bgTag == bgTag)
	{
		_peerSocket->async_send(asio::buffer(message->messageBuf.data(), message->messageBuf.size()),
			std::bind(&Peer::_sendMssgFuncFeedbk, this, std::placeholders::_1, std::placeholders::_2));

	}
}

void Peer::disconnect()
{
	DEBUG_LOG(Log::ALog(_saPair.toString(), " Peer Disconnecting");)
	_peerIsActive = false;
}

void Peer::listenTo(const std::string_view& bgID, const std::string_view& bgTag)
{
	if (_isInBG)
		_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::IS_LISTENING];
	else
	{
		std::lock_guard<std::mutex> lock(_resLock);
		_bgID = bgID;
		_bgTag = bgTag;
		
		_bgPtr = BGcontroller::addToBG(this, _bgID);
		if (_bgPtr != nullptr)
		{
			_isInBG = true;
			auto message = Message::makeAddMsg(_saPair);
			if (message != nullptr)
				_bgPtr->broadcast(this, _bgTag, message);

			_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
			DEBUG_LOG(Log::ALog(_saPair.toString(), " Listening to Tag: ", _bgTag, " BG: ", _bgID);)
		}
		else
			_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::WAIT_RETRY];
	}
}

void Peer::leaveBG()
{
	if (_isInBG)
	{
		DEBUG_LOG(Log::ALog(_saPair.toString(), " Peer leavig BG ", _bgID);)
		BGcontroller::removeFromBG(this, _bgID);
		auto message = Message::makeRemMsg(_saPair);
		if (message != nullptr)
			((BGroup*)_bgPtr)->broadcast(this, _bgTag, message);

		_isInBG = false;
		_bgPtr = nullptr;
		_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
	}
	else
		_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::NOT_LISTENING];
}

void Peer::changeTagTo(const std::string_view& bgTag)
{
	if (_isInBG)
	{
		std::lock_guard<std::mutex> lock(_resLock);
		_bgTag = bgTag;
		_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::ALog(_saPair.toString(), " Peer Changing tag to ", _bgTag);)
	}
	else
		_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::NOT_LISTENING];
}

void Peer::printPingInfo()
{
	DEBUG_LOG(Log::ALog(_saPair.toString(), " Peer pinging");)
	_writeBuffer += "[R] " + _saPair.toString();
}

void Peer::respondWith(Response response)
{
	DEBUG_LOG(Log::ALog(_saPair.toString(), " Peer responding: ", CmdProcessor::RESP[(short)response]);)
	_writeBuffer += "[R] " + CmdProcessor::RESP[(short)response];
}

void Peer::broadcast(const std::string_view& messageStr)
{
	if (_isInBG)
	{
		auto message = Message::makeBrdMsg(_saPair, messageStr);
		if (message != nullptr)
			_bgPtr->broadcast(this, _bgTag, message);

		_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::ALog(_saPair.toString(), " Peer broadcasting: ", messageStr);)
	}
	else
		_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::NOT_LISTENING];
}

Peer::~Peer()
{
	leaveBG();
	asio::error_code ec;
	_peerSocket->close(ec);
	if (ec)
		LOG(Log::log("TCP Socket cannot close", _peerSocket, ec);)

	_peerCount--;
	delete _peerSocket;
	DEBUG_LOG(Log::ALog("Peer Disconnected: " + _saPair.toString());)
}


void Peer::_terminatePeer()
{
	DEBUG_LOG(Log::ALog(_saPair.toString(), " Peer terminating");)
	_peerSocket->shutdown(asio::ip::tcp::socket::shutdown_both);
	delete this;
}

void Peer::_sendFuncFeedbk(const asio::error_code& ec, std::size_t size)
{
	if (ec)
	{
		LOG(Log::log("TCP Socket _sendData() failed", _peerSocket, ec);)
		_terminatePeer();
	}
	else
	{
		_writeBuffer.clear();
		_peerReceiveData();
	}
}

void Peer::_sendMssgFuncFeedbk(const asio::error_code& ec, std::size_t size)
{
	if (ec)
	{
		LOG(Log::log("TCP Socket _sendMessage() failed", _peerSocket, ec);)
		_peerIsActive = false;
	}
}

void Peer::_sendPeerBufferData()
{
	_peerSocket->async_send(asio::buffer(_writeBuffer.data(), _writeBuffer.size()), std::bind(&Peer::_sendFuncFeedbk,
		this, std::placeholders::_1, std::placeholders::_2));
}

void Peer::_peerReceiveData()
{
	if (_peerIsActive)
	{
		_peerSocket->async_receive(asio::buffer(_dataBuffer.data(), RTDS_BUFF_SIZE), 0, std::bind(&Peer::_processData,
			this, std::placeholders::_1, std::placeholders::_2));
	}
	else
		_terminatePeer();
}

void Peer::_processData(const asio::error_code& ec, std::size_t size)
{
	if (ec)
	{
		LOG(Log::log("TCP Socket _processData() failed", _peerSocket, ec);)
		_terminatePeer();
	}
	else
	{
		_dataBuffer[size] = '\0';
		commandStr = (char*)_dataBuffer.data();
		DEBUG_LOG(Log::ALog(_saPair.toString(), " Peer received: ",commandStr);)
		CmdProcessor::processCommand(*this);

		if (_peerIsActive)
			_sendPeerBufferData();
		else
			_terminatePeer();
	}
}