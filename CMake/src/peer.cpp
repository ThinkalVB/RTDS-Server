#include "peer.h"
#include "cmd_processor.h"
#include "bg_controller.h"
#include "log.h"

std::atomic_int Peer::m_peerCount = 0;

Peer::Peer(asio::ip::tcp::socket* socketPtr) : m_saPair(socketPtr)
{
	m_peerSocket = socketPtr;
	m_writeBuffer.reserve(RTDS_BUFF_SIZE);

	m_peerIsActive = true;
	m_bgPtr = nullptr;
	m_isInBG = false;

	DEBUG_LOG(Log::log(m_saPair.toString()," Peer Connected");)
	m_peerCount++;
	m_writeBuffer += m_saPair.toString();
	m_sendPeerBufferData();
}

int Peer::peerCount()
{
	return m_peerCount;
}

Peer::~Peer()
{
	leaveBG();
	m_peerCount--;
	m_peerSocket->shutdown(asio::ip::tcp::socket::shutdown_both);

	asio::error_code ec;
	m_peerSocket->close(ec);
	if (ec)
	{	LOG(Log::log(m_saPair.toString(), " socket cannot close - ", ec.message());)		}
	delete m_peerSocket;
	DEBUG_LOG(Log::log(m_saPair.toString(), " Peer Disconnected");)
}


void Peer::sendMessage(const Message* message, const std::string_view& bgTag)
{
	if (m_peerIsActive && m_bgTag == bgTag)
	{
		m_peerSocket->async_send(asio::buffer(message->messageBuf.data(), message->messageBuf.size()),
			std::bind(&Peer::m_sendMssgFuncFeedbk, this, std::placeholders::_1, std::placeholders::_2));
	}
}

void Peer::sendMessage(const Message* message)
{
	if (m_peerIsActive)
	{
		m_peerSocket->async_send(asio::buffer(message->messageBuf.data(), message->messageBuf.size()),
			std::bind(&Peer::m_sendMssgFuncFeedbk, this, std::placeholders::_1, std::placeholders::_2));
	}
}


void Peer::m_sendFuncFeedbk(const asio::error_code& ec, std::size_t size)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(m_saPair.toString(), " Peer socket _sendData() failed", ec.message());)
		delete this;
	}
	else
	{
		m_writeBuffer.clear();
		m_peerReceiveData();
	}
}

void Peer::m_sendMssgFuncFeedbk(const asio::error_code& ec, std::size_t size)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(m_saPair.toString(), " Peer socket _sendMessage() failed", ec.message());)
		m_peerIsActive = false;
	}
}


void Peer::m_sendPeerBufferData()
{
	m_peerSocket->async_send(asio::buffer(m_writeBuffer.data(), m_writeBuffer.size()), std::bind(&Peer::m_sendFuncFeedbk,
		this, std::placeholders::_1, std::placeholders::_2));
}

void Peer::m_peerReceiveData()
{
	if (m_peerIsActive)
	{
		m_peerSocket->async_receive(asio::buffer(m_dataBuffer.data(), RTDS_BUFF_SIZE), 0, std::bind(&Peer::m_processData,
			this, std::placeholders::_1, std::placeholders::_2));
	}
	else
		delete this;
}

void Peer::m_processData(const asio::error_code& ec, std::size_t size)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(m_saPair.toString(), " Peer socket _processData() failed ", ec.message());)
		delete this;
	}
	else
	{
		m_dataBuffer[size] = '\0';
		commandStr = (char*)m_dataBuffer.data();

		DEBUG_LOG(Log::log(m_saPair.toString(), " Peer received: ",commandStr);)
		CmdProcessor::processCommand(*this);

		if (m_peerIsActive)
			m_sendPeerBufferData();
		else
			delete this;
	}
}


void Peer::disconnect()
{
	DEBUG_LOG(Log::log(m_saPair.toString(), " Peer Disconnecting");)
	m_peerIsActive = false;
}

void Peer::listenTo(const std::string_view& bgID, const std::string_view& bgTag)
{
	if (m_isInBG)
		m_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::IS_IN_BG];
	else
	{
		m_bgID = bgID;
		m_bgTag = bgTag;

		m_bgPtr = BGcontroller::addToBG(this, m_bgID);
		if (m_bgPtr != nullptr)
		{
			m_isInBG = true;
			auto message = Message::makeAddMsg(m_saPair, m_bgTag);
			if (message != nullptr)
				m_bgPtr->broadcast(this, message);

			m_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
			DEBUG_LOG(Log::log(m_saPair.toString(), " Listening to Tag: ", m_bgTag, " BG: ", m_bgID);)
		}
		else
			m_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::WAIT_RETRY];
	}
}

void Peer::leaveBG()
{
	if (m_isInBG)
	{
		DEBUG_LOG(Log::log(m_saPair.toString(), " Peer leavig BG ", m_bgID);)
		BGcontroller::removeFromBG(this, m_bgID);
		auto message = Message::makeRemMsg(m_saPair, m_bgTag);
		if (message != nullptr)
			m_bgPtr->broadcast(this, message);

		m_isInBG = false;
		m_bgPtr = nullptr;
		m_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
	}
	else
		m_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::NOT_IN_BG];
}

void Peer::printPingInfo()
{
	DEBUG_LOG(Log::log(m_saPair.toString(), " Peer pinging");)
	m_writeBuffer += "[R] " + m_saPair.toString();
}

void Peer::respondWith(Response response)
{
	DEBUG_LOG(Log::log(m_saPair.toString(), " Peer responding: ", CmdProcessor::RESP[(short)response]);)
	m_writeBuffer += "[R] " + CmdProcessor::RESP[(short)response];
}

void Peer::broadcast(const std::string_view& messageStr)
{
	if (m_isInBG)
	{
		auto message = Message::makeBrdMsg(m_saPair, messageStr);
		if (message != nullptr)
			m_bgPtr->broadcast(this, message);

		m_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::log(m_saPair.toString(), " Peer broadcasting: ", messageStr);)
	}
	else
		m_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::NOT_IN_BG];
}

void Peer::broadcast(const std::string_view& messageStr, const std::string_view& bgTag)
{
	if (m_isInBG)
	{
		auto message = Message::makeBrdMsg(m_saPair, messageStr);
		if (message != nullptr)
			m_bgPtr->broadcast(this, message, bgTag);

		m_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::log(m_saPair.toString(), " Peer broadcasting: ", messageStr);)
	}
	else
		m_writeBuffer += "[R] " + CmdProcessor::RESP[(short)Response::NOT_IN_BG];
}
