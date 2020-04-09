#include "CmdInterpreter.h"

void CmdInterpreter::processCommand(Peer& peer)
{
	auto commandString = std::string_view{ peer.dataBuffer };
	if (commandString.find(' ') == std::string::npos)
	{
		if (commandString == "ping")
			CmdInterpreter::ping(peer);
		else
			peer.writeBuffer = "bad_command";
	}
	else
	{
		peer.writeBuffer = "bad_command";
	}
	peer._sendPeerData();
}

void CmdInterpreter::ping(Peer& peer)
{
	auto peerIpAddress = peer.peerSocket->remote_endpoint().address();
	auto peerPortNumber = peer.peerSocket->remote_endpoint().port();
	if (peerIpAddress.is_v4())
		peer.writeBuffer = "v4 ";
	else
		peer.writeBuffer = "v6 ";
	peer.writeBuffer = peer.writeBuffer + peerIpAddress.to_string() + " " + std::to_string(peerPortNumber);
}
