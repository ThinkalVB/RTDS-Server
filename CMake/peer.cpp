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

	DEBUG_LOG(Log::log(_saPair.toString()," Peer Connected");)
	_peerCount++;
	_writeBuffer += _saPair.toString();
	_sendPeerBufferData();
}

Peer::~Peer()
{
	leaveBG();
	_peerCount--;
	_peerSocket->shutdown(asio::ip::tcp::socket::shutdown_both);

	asio::error_code ec;
	_peerSocket->close(ec);
	if (ec)
		LOG(Log::log(_saPair.toString(), " socket cannot close - ", ec.message());)
	delete _peerSocket;
	DEBUG_LOG(Log::log(_saPair.toString(), " Peer Disconnected");)
}


void Peer::sendMessage(const Message* message, const std::string_view& bgTag)
{
	if (_peerIsActive && _bgTag == bgTag)
	{
		_peerSocket->async_send(asio::buffer(message->messageBuf.data(), message->messageBuf.size()),
			std::bind(&Peer::_sendMssgFuncFeedbk, this, std::placeholders::_1, std::placeholders::_2));
	}
}

void Peer::sendMessage(const Message* message)
{
	if (_peerIsActive)
	{
		_peerSocket->async_send(asio::buffer(message->messageBuf.data(), message->messageBuf.size()),
			std::bind(&Peer::_sendMssgFuncFeedbk, this, std::placeholders::_1, std::placeholders::_2));
	}
}


void Peer::_sendFuncFeedbk(const asio::error_code& ec, std::size_t size)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(_saPair.toString(), " Peer socket _sendData() failed", ec.message());)
		delete this;
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
		DEBUG_LOG(Log::log(_saPair.toString(), " Peer socket _sendMessage() failed", ec.message());)
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
		delete this;
}

void Peer::_processData(const asio::error_code& ec, std::size_t size)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(_saPair.toString(), " Peer socket _processData() failed ", ec.message());)
		delete this;
	}
	else
	{
		_dataBuffer[size] = '\0';
		commandStr = (char*)_dataBuffer.data();

		DEBUG_LOG(Log::log(_saPair.toString(), " Peer received: ",commandStr);)
		CmdProcessor::processCommand(*this);

		if (_peerIsActive)
			_sendPeerBufferData();
		else
			delete this;
	}
}


void Peer::disconnect()
{
	DEBUG_LOG(Log::log(_saPair.toString(), " Peer Disconnecting");)
	_peerIsActive = false;
}

void Peer::listenTo(const std::string_view& bgID, const std::string_view& bgTag)
{
	if (_isInBG)
		_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::IS_IN_BG];
	else
	{
		_bgID = bgID;
		_bgTag = bgTag;

		_bgPtr = BGcontroller::addToBG(this, _bgID);
		if (_bgPtr != nullptr)
		{
			_isInBG = true;
			auto message = Message::makeAddMsg(_saPair, _bgTag);
			if (message != nullptr)
				_bgPtr->broadcast(this, message);

			_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
			DEBUG_LOG(Log::log(_saPair.toString(), " Listening to Tag: ", _bgTag, " BG: ", _bgID);)
		}
		else
			_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::WAIT_RETRY];
	}
}

void Peer::leaveBG()
{
	if (_isInBG)
	{
		DEBUG_LOG(Log::log(_saPair.toString(), " Peer leavig BG ", _bgID);)
		BGcontroller::removeFromBG(this, _bgID);
		auto message = Message::makeRemMsg(_saPair, _bgTag);
		if (message != nullptr)
			_bgPtr->broadcast(this, message);

		_isInBG = false;
		_bgPtr = nullptr;
		_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
	}
	else
		_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::NOT_IN_BG];
}

void Peer::printPingInfo()
{
	DEBUG_LOG(Log::log(_saPair.toString(), " Peer pinging");)
	_writeBuffer += "[R] " + _saPair.toString();
}

void Peer::respondWith(Response response)
{
	DEBUG_LOG(Log::log(_saPair.toString(), " Peer responding: ", CmdProcessor::RESP[(short)response]);)
	_writeBuffer += "[R] " + CmdProcessor::RESP[(short)response];
}

void Peer::broadcast(const std::string_view& messageStr)
{
	if (_isInBG)
	{
		auto message = Message::makeBrdMsg(_saPair, messageStr);
		if (message != nullptr)
			_bgPtr->broadcast(this, message);

		_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::log(_saPair.toString(), " Peer broadcasting: ", messageStr);)
	}
	else
		_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::NOT_IN_BG];
}

void Peer::broadcast(const std::string_view& messageStr, const std::string_view& bgTag)
{
	if (_isInBG)
	{
		auto message = Message::makeBrdMsg(_saPair, messageStr);
		if (message != nullptr)
			_bgPtr->broadcast(this, message, bgTag);

		_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::log(_saPair.toString(), " Peer broadcasting: ", messageStr);)
	}
	else
		_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::NOT_IN_BG];
}
