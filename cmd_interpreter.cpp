#include "cmd_interpreter.h"
#include "log.h"

void CmdInterpreter::processCommand(Peer& peer)
{
	auto commandString = std::string_view{ peer.dataBuffer };
	if (commandString.find(' ') == std::string::npos)
	{
		if (commandString == Command::COM_PING)
			CmdInterpreter::ping(peer);
		else
			peer.writeBuffer = Response::BAD_COMMAND;
	}
	else
	{
		peer.writeBuffer = Response::BAD_COMMAND;
	}
	peer._sendPeerData();
}

void CmdInterpreter::ping(Peer& peer)
{
	if (peer.remoteEp.address().is_v4())
	{
		peer.writeBuffer += peer.peerEntry.Ev4->versionID + " ";
		peer.writeBuffer += peer.peerEntry.Ev4->ipAddress + " ";
		peer.writeBuffer += peer.peerEntry.Ev4->portNumber;
	}
	else
	{
		peer.writeBuffer += peer.peerEntry.Ev6->versionID + " ";
		peer.writeBuffer += peer.peerEntry.Ev6->ipAddress + " ";
		peer.writeBuffer += peer.peerEntry.Ev6->portNumber;
	}
}

void CmdInterpreter::makeSourcePair(const asio::ip::address_v4& ipAddress, unsigned short portNum, sourcePairV4& sourcePair)
{
	auto ipBin = ipAddress.to_bytes();
	memcpy(&sourcePair[0], &ipBin[0], 4);

	#ifdef BOOST_ENDIAN_LITTLE_BYTE
	byteSwap(portNum);
	#endif
	memcpy(&sourcePair[4], &portNum, 2);
}

void CmdInterpreter::makeSourcePair(const asio::ip::address_v6& ipAddress, unsigned short portNum, sourcePairV6& sourcePair)
{
	auto ipBin = ipAddress.to_bytes();
	memcpy(&sourcePair[0], &ipBin[0], 16);

	#ifdef BOOST_ENDIAN_LITTLE_BYTE
	byteSwap(portNum);
	#endif
	memcpy(&sourcePair[16], &portNum, 2);
}