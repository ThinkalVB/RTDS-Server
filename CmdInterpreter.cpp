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
		peer.writeBuffer = "v4 ";
	else
		peer.writeBuffer = "v6 ";
	peer.writeBuffer = peer.writeBuffer + peer.remoteEp.address().to_string() +
		" " + std::to_string(peer.remoteEp.port());
}

void CmdInterpreter::makeSourcePairV4(asio::ip::tcp::endpoint& remoteEp, unsigned short portNum, uint8_t(&sourcePair)[18])
{
	auto addressClass = remoteEp.address().to_v4();
	auto ipBin = addressClass.to_bytes();
	memcpy(&sourcePair[0], &ipBin[0], 4);
	
	#ifdef BOOST_ENDIAN_LITTLE_BYTE
	byteSwap(portNum);
	#endif
	memcpy(&sourcePair[4], &portNum, 2);
}

void CmdInterpreter::makeSourcePairV6(asio::ip::tcp::endpoint& remoteEp, unsigned short portNum, uint8_t(&sourcePair)[18])
{
	auto addressClass = remoteEp.address().to_v6();
	auto ipBin = addressClass.to_bytes();
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


