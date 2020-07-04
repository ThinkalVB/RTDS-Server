#include "udp_peer.h"
#include <functional>
#include "cmd_processor.h"
#include "bg_controller.h"
#include "log.h"

asio::ip::udp::socket* UDPpeer::mPeerSocket;

void UDPpeer::mSendPeerBufferData()
{
	mPeerSocket->send_to(getSendBuffer(), mUDPep);
}

asio::ip::udp::endpoint& UDPpeer::getRefToEndpoint()
{
	return mUDPep;
}

UDPpeer::UDPpeer(asio::ip::udp::socket* udpSocket)
{
	mPeerSocket = udpSocket;
}


void UDPpeer::printPingInfo()
{
	auto saPair = SApair(mUDPep);
	std::string response = "[R]\t";
	response += saPair.toString();
	response += "\n";
	mDataBuffer = response;

	DEBUG_LOG(Log::log("UDP Peer pinging");)
	mSendPeerBufferData();
}

void UDPpeer::respondWith(const Response resp)
{
	std::string response = "[R]\t";
	response += CmdProcessor::RESP[(short)resp];
	response += "\n";
	mDataBuffer = response;

	DEBUG_LOG(Log::log("UDP Peer responding: ", CmdProcessor::RESP[(short)resp]);)
	mSendPeerBufferData();
}

void UDPpeer::broadcast(const std::string_view& messageStr, const std::string_view& bgID, const std::string_view& bgTag)
{
	auto saPair = SApair(mUDPep);
	std::string response = "[R]\t";
	auto message = Message::makeBrdMsg(saPair.toString(), messageStr, UDP_TAG, bgTag, PeerType::UDP);

	if (message != nullptr)
	{
		BGcontroller::broadcast(message, std::string(bgID));
		response += CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::log("Peer broadcasting: ", messageStr);)
	}
	else
	{
		response += CmdProcessor::RESP[(short)Response::WAIT_RETRY];
		LOG(Log::log("Failed to create UDP message!");)
	}

	response += "\n";
	mDataBuffer = response;
	mSendPeerBufferData();
}
