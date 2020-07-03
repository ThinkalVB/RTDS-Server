#include "udp_peer.h"
#include <functional>
#include "cmd_processor.h"
#include "bg_controller.h"
#include "log.h"

asio::ip::udp::socket* UDPpeer::mPeerSocket;

void UDPpeer::mSendMssgFuncFeedbk(const asio::error_code& ec)
{
	DEBUG_LOG(	if (ec)
					Log::log("UDP message sending failed", ec.message());	)
}

void UDPpeer::registerUDPsocket(asio::ip::udp::socket* udpSock)
{
	mPeerSocket = udpSock;
}

asio::ip::udp::endpoint& UDPpeer::getRefToEp()
{
	return mUDPep;
}

const std::string UDPpeer::getSApairString() const
{
	std::string saPairStr;
	auto ipAddr = mUDPep.address();
	auto portNumber = mUDPep.port();

	if (ipAddr.is_v4())
	{
		saPairStr = STR_V4;
		auto _ipAddr4 = ipAddr.to_v4();
		saPairStr += "\t" + _ipAddr4.to_string();
	}
	else
	{
		saPairStr = STR_V6;
		auto _ipAddr6 = ipAddr.to_v6();
		if (_ipAddr6.is_v4_mapped())
			saPairStr += "\t" + _ipAddr6.to_v4().to_string();
		else
			saPairStr += "\t" + _ipAddr6.to_string();
	}
	saPairStr += "\t" + std::to_string(portNumber);
	return saPairStr;
}


void UDPpeer::sendMessage(const Message* message, const std::string_view& bgTag)
{
	if (mBgTag == bgTag)
	{
		mPeerSocket->async_send_to(asio::buffer(message->messageBuf.data(), message->messageBuf.size()), mUDPep,
			std::bind(&UDPpeer::mSendMssgFuncFeedbk, std::placeholders::_1));
	}
}

void UDPpeer::sendMessage(const Message* message)
{
	mPeerSocket->async_send_to(asio::buffer(message->messageBuf.data(), message->messageBuf.size()), mUDPep,
		std::bind(&UDPpeer::mSendMssgFuncFeedbk, std::placeholders::_1));
}


void UDPpeer::printPingInfo(AdancedBuffer& dataBuffer) const
{
	std::string response = "[R]\t";
	response += getSApairString();
	response += "\n";
	dataBuffer = response;

	DEBUG_LOG(Log::log("UDP Peer pinging");)
}

void UDPpeer::respondWith(const Response resp, AdancedBuffer& dataBuffer) const
{
	std::string response = "[R]\t";
	response += CmdProcessor::RESP[(short)resp];
	response += "\n";
	dataBuffer = response;

	DEBUG_LOG(Log::log("UDP Peer responding: ", CmdProcessor::RESP[(short)resp]);)

}

void UDPpeer::broadcast(const std::string_view& messageStr, const std::string_view& bgID, AdancedBuffer& dataBuffer) const
{
	/*
	std::string response = "[R]\t";
	auto message = Message::makeBrdMsg(getSApairString(), messageStr, ALL_TAG);
	
	if (message != nullptr)
	{
		BGcontroller::broadcast(message, std::string(bgID));
		response += CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::log("Peer broadcasting: ", messageStr);)
	}
	else
		response += CmdProcessor::RESP[(short)Response::WAIT_RETRY];
	response += "\n";
	dataBuffer = response;
	*/
}

void UDPpeer::broadcast(const std::string_view& messageStr, const std::string_view& bgID, const std::string_view& bgTag, AdancedBuffer& dataBuffer) const
{
	/*
	std::string response = "[R]\t";
	auto message = Message::makeBrdMsg(getSApairString(), messageStr, ALL_TAG);

	if (message != nullptr)
	{
		BGcontroller::broadcast(message, std::string(bgID), bgTag);
		response += CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::log("Peer broadcasting: ", messageStr);)
	}
	else
		response += CmdProcessor::RESP[(short)Response::WAIT_RETRY];
	response += "\n";
	dataBuffer = response;
	*/
}
