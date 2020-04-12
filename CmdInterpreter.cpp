#include "CmdInterpreter.h"
#include "Log.h"

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
	if (peer.remoteEp.address().is_v4())
	{
		peer.writeBuffer += peer.peerEntry.SPv4->versionID + " ";
		peer.writeBuffer += peer.peerEntry.SPv4->ipAddress + " ";
		peer.writeBuffer += peer.peerEntry.SPv4->portNumber;
	}
	else
	{
		peer.writeBuffer += peer.peerEntry.SPv6->versionID + " ";
		peer.writeBuffer += peer.peerEntry.SPv6->ipAddress + " ";
		peer.writeBuffer += peer.peerEntry.SPv6->portNumber;
	}
}

void CmdInterpreter::makeSourcePairV4(const asio::ip::address_v4& ipAddress, unsigned short portNum, uint8_t(&sourcePair)[6])
{
	auto ipBin = ipAddress.to_bytes();
	memcpy(&sourcePair[0], &ipBin[0], 4);

	#ifdef BOOST_ENDIAN_LITTLE_BYTE
	byteSwap(portNum);
	#endif
	memcpy(&sourcePair[4], &portNum, 2);
}

void CmdInterpreter::makeSourcePairV6(const asio::ip::address_v6& ipAddress, unsigned short portNum, uint8_t(&sourcePair)[18])
{
	auto ipBin = ipAddress.to_bytes();
	memcpy(&sourcePair[0], &ipBin[0], 16);

	#ifdef BOOST_ENDIAN_LITTLE_BYTE
	byteSwap(portNum);
	#endif
	memcpy(&sourcePair[16], &portNum, 2);
}

bool CmdInterpreter::validIPaddress(std::string ipAddress, unsigned short portNum)
{
	system::error_code ec;
	asio::ip::make_address(ipAddress, ec);
	if (ec != system::errc::success)
	{
		#ifdef PRINT_LOG
		Log::log("Bad IP address");
		#endif
		return false;
	}
	else
	{
		if (portNum <= 65535)
			return true;
		else
			return false;
	}
}