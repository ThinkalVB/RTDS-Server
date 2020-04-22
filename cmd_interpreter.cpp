#include "cmd_interpreter.h"
#include <charconv>
#include "directory.h"
#include <regex>
#include "log.h"

using namespace boost;
#define V4_UID_MAX_CHAR 8
#define V6_UID_MAX_CHAR 24
#define PORT_NUM_MAX_CHAR 5
#define MAX_PORT_NUM_VALUE 65535

const std::string CmdInterpreter::RESP[] = 
{
	"redudant_data",
	"ok_success",
	"no_privilege",
	"bad_command",
	"bad_param",
	"no_exist",
	"wait_retry",
};

const std::string CmdInterpreter::COMM[] =
{
	"ping",
	"add",
	"search",
	"ttl",
	"charge",
	"flush",
	"update",
	"remove",
	"count",
	"mirror",
	"leave",
	"exit"
};

bool CmdInterpreter::_extractElement(std::string_view& commandLine, std::string_view& element)
{
	auto cmdSize = commandLine.size();
	auto endIndex = commandLine.find_first_not_of(' ');
	commandLine.remove_prefix(std::min(endIndex, cmdSize));

	cmdSize = commandLine.size();
	if (cmdSize > 0)
	{
		if (commandLine[0] == '[')
		{
			endIndex = commandLine.find_last_of(']');
			if (endIndex == std::string::npos)
			{
				commandLine.remove_prefix(cmdSize);
				return false;
			}
			else
			{
				element = commandLine.substr(0, endIndex + 1);
				commandLine.remove_prefix(cmdSize);
				return true;
			}
		}
		else
		{
			endIndex = commandLine.find(' ');
			if (endIndex == std::string::npos)
			{
				element = commandLine;
				commandLine.remove_prefix(cmdSize);
			}
			else
			{
				element = commandLine.substr(0, endIndex);
				commandLine.remove_prefix(endIndex);
			}
			return true;

		}
	}
	else
		return false;
}

bool CmdInterpreter::_isBase64(const std::string_view& uid)
{
	std::regex regx("[a-zA-Z0-9\+/]*");
	return std::regex_match(uid.cbegin(), uid.cend(), regx);
}

bool CmdInterpreter::_validPortNumber(const std::string_view& portNumStr, unsigned short& portNum)
{
	std::regex regx("[0-9]{1,5}");
	if (std::regex_match(portNumStr.cbegin(), portNumStr.cend(), regx))
	{
		int portNumber;
		std::from_chars(portNumStr.data(), portNumStr.data() + portNumStr.size(), portNumber);
		if (portNumber <= MAX_PORT_NUM_VALUE)
		{
			portNum = (unsigned short)portNumber;
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

bool CmdInterpreter::_makeSourcePair(const std::string_view& ipAddrStr, const std::string_view& portNum, SourcePair& sourcePair)
{
	system::error_code ec;
	auto ipAddress = asio::ip::make_address(ipAddrStr, ec);
	unsigned short portNumber;
	if (ec == system::errc::success && _validPortNumber(portNum, portNumber))
	{
		if (ipAddress.is_v4())
		{
			makeSourcePair(ipAddress.to_v4(), portNumber, sourcePair.SP.V4);
			sourcePair.version = Version::V4;
		}
		else
		{
			makeSourcePair(ipAddress.to_v6(), portNumber, sourcePair.SP.V6);
			sourcePair.version = Version::V6;
		}
		return true;
	}
	else
		return false;

}

bool CmdInterpreter::_makeSourcePair(const std::string_view& uid, SourcePair& sourcePair)
{
	if (_isBase64(uid))
	{
		if (uid.size() == V4_UID_MAX_CHAR)
		{
			cppcodec::base64_rfc4648::decode(sourcePair.SP.V4.data(), sourcePair.SP.V4.size(), uid);
			sourcePair.version = Version::V4;
		}
		else if (uid.size() == V6_UID_MAX_CHAR)
		{
			cppcodec::base64_rfc4648::decode(sourcePair.SP.V6.data(), sourcePair.SP.V6.size(), uid);
			sourcePair.version = Version::V6;
		}
		else
			return false;
		return true;
	}
	else
		return false;
}

void CmdInterpreter::processCommand(Peer& peer)
{
	std::string_view command;
	if (_extractElement(peer.receivedData, command))
	{
		if (command == COMM[(short)Command::PING])
			s_ping(peer);
		else if (command == COMM[(short)Command::COUNT])
			s_count(peer);
		else if (command == COMM[(short)Command::MIRROR])
			s_mirror(peer);
		else if (command == COMM[(short)Command::LEAVE])
			s_leave(peer);
		else if (command == COMM[(short)Command::ADD])
			_add(peer);
		else if (command == COMM[(short)Command::SEARCH])
			_search(peer);
		else if (command == COMM[(short)Command::CHARGE])
			_charge(peer);
		else if (command == COMM[(short)Command::TTL])
			_ttl(peer);
		else if (command == COMM[(short)Command::REMOVE])
			_remove(peer);
		else if (command == COMM[(short)Command::FLUSH])
			_flush(peer);
		else if (command == COMM[(short)Command::UPDATE])
			_update(peer);
		else if (command == COMM[(short)Command::EXIT])
		{
			peer.peerSocket->shutdown(asio::ip::tcp::socket::shutdown_both);
			return;
		}
	}
	if (peer.writeBuffer.size() == 0)
		peer.writeBuffer = CmdInterpreter::RESP[(short)Response::BAD_COMMAND];
	peer._sendPeerData();
}


/* Commands without any arguments				*/		
void CmdInterpreter::s_ping(Peer& peer)
{
	peer.writeBuffer += RESP[(short)Response::SUCCESS] + " ";
	peer.peerEntry.Ev->printBrief(peer.writeBuffer);
}

void CmdInterpreter::s_count(Peer& peer)
{
	peer.writeBuffer += RESP[(short)Response::SUCCESS] + " ";
	peer.writeBuffer += std::to_string(Directory::getEntryCount());
}

void CmdInterpreter::s_mirror(Peer& peer)
{
	peer.writeBuffer += RESP[(short)Response::SUCCESS];
	peer.addToMirroringGroup();
}

void CmdInterpreter::s_leave(Peer& peer)
{
	peer.writeBuffer += RESP[(short)Response::SUCCESS];
	peer.removeFromMirroringGroup();
}

/* Commands with arguments						*/
void CmdInterpreter::_ttl(Peer& peer)
{
	std::string_view uid, portNum;
	auto ipAddress = std::ref(uid);

	Response response = Response::BAD_PARAM;
	short ttl;

	if (_extractElement(peer.receivedData, uid))
	{
		SourcePair sourcePair;
		if (_isBase64(uid))
		{
			if (_makeSourcePair(uid, sourcePair))
				response = Directory::getTTL(sourcePair, ttl);
		}
		else if (_extractElement(peer.receivedData, portNum))
		{
			if (_makeSourcePair(ipAddress, portNum, sourcePair))
				response = Directory::getTTL(sourcePair, ttl);
		}
	}
	else
		response = Directory::getTTL(peer.peerEntry.Ev, ttl);

	peer.writeBuffer += RESP[(short)response];
	if (response == Response::SUCCESS)
		peer.writeBuffer += " " + std::to_string(ttl);
}

void CmdInterpreter::_remove(Peer& peer)
{
	std::string_view uid, portNum;
	auto ipAddress = std::ref(uid);

	Response response = Response::BAD_PARAM;
	if (_extractElement(peer.receivedData, uid))
	{
		SourcePair sourcePair;
		if (_isBase64(uid))
		{
			if (_makeSourcePair(uid, sourcePair))
				response = Directory::removeFromDir(sourcePair, peer.peerEntry.Ev);
		}
		else if (_extractElement(peer.receivedData, portNum))
		{
			if (_makeSourcePair(ipAddress, portNum, sourcePair))
				response = Directory::removeFromDir(sourcePair, peer.peerEntry.Ev);
		}
	}
	else
		response = Directory::removeFromDir(peer.peerEntry.Ev);
	peer.writeBuffer += RESP[(short)response];
}

void CmdInterpreter::_flush(Peer& peer)
{
}

void CmdInterpreter::_update(Peer& peer)
{
}

void CmdInterpreter::_add(Peer& peer)
{
	auto response = Directory::addToDir(peer.peerEntry.Ev);
	peer.writeBuffer += RESP[(short)response] + " ";
	peer.peerEntry.Ev->printUID(peer.writeBuffer);
}

void CmdInterpreter::_search(Peer& peer)
{
}

void CmdInterpreter::_charge(Peer& peer)
{
}