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
#define MAX_DESC_SIZE 202

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

bool CmdInterpreter::populateElement(Peer& peer, const size_t& size)
{
	std::string_view commandLine;
	if (peer.dataBuffer[size - 1] == ';')
	{
		peer.dataBuffer[size - 1] = '\0';
		commandLine = peer.dataBuffer;
		peer.cmdElement.reset();
	}
	else
		return false;

	while (!commandLine.empty())
	{
		if (peer.cmdElement.size() == 5)
			return false;

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
					return false;
				else
				{
					peer.cmdElement.push_back(commandLine.substr(0, endIndex + 1));
					commandLine.remove_prefix(cmdSize);
				}
			}
			else
			{
				endIndex = commandLine.find(' ');
				if (endIndex == std::string::npos)
				{
					peer.cmdElement.push_back(commandLine);
					break;
				}
				else
				{
					peer.cmdElement.push_back(commandLine.substr(0, endIndex));
					commandLine.remove_prefix(endIndex);
				}
			}
		}
		else
			break;
	}

	if (peer.cmdElement.size() > 0)
	{
		peer.cmdElement.reset_for_read();
		return true;
	}
	else
		return false;
}

bool CmdInterpreter::_isBase64(const std::string_view& uid)
{
	std::regex regx("[a-zA-Z0-9\+/]*");
	return std::regex_match(uid.cbegin(), uid.cend(), regx);
}

bool CmdInterpreter::_isDescription(const std::string_view& desc)
{
	if (desc[0] == '[' && desc[desc.size() - 1] == ']' && desc.size() <= MAX_DESC_SIZE)
		return true;
	return false;
}

bool CmdInterpreter::_isPermission(const std::string_view& permission)
{
	std::regex regx("[plr]{3}");
	return std::regex_match(permission.cbegin(), permission.cend(), regx);
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
	auto command = peer.cmdElement.pop_front();
	if (command == COMM[(short)Command::PING] && peer.cmdElement.size() == 0)
		s_ping(peer);
	else if (command == COMM[(short)Command::COUNT] && peer.cmdElement.size() == 0)
		s_count(peer);
	else if (command == COMM[(short)Command::MIRROR] && peer.cmdElement.size() == 0)
		s_mirror(peer);
	else if (command == COMM[(short)Command::LEAVE] && peer.cmdElement.size() == 0)
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
	else if (command == COMM[(short)Command::EXIT] && peer.cmdElement.size() == 0)
	{
		peer.terminatePeer();
		return;
	}

	if (peer.writeBuffer.size() == 0)
		peer.writeBuffer = CmdInterpreter::RESP[(short)Response::BAD_COMMAND];
	peer._sendPeerData();
}

/* Commands without any arguments				*/		
void CmdInterpreter::s_ping(Peer& peer)
{
	peer.writeBuffer += RESP[(short)Response::SUCCESS] + " ";
	peer.entry()->printBrief(peer.writeBuffer);
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
	Response response = Response::BAD_PARAM;
	SourcePair sourcePair;
	short ttl;

	if (peer.cmdElement.size() == 0)
		response = Directory::getTTL(peer.entry(), ttl);
	else if (peer.cmdElement.size() == 1)
	{
		if (_isBase64(peer.cmdElement.peek()) && _makeSourcePair(peer.cmdElement.peek(), sourcePair))
			response = Directory::getTTL(sourcePair, ttl);
	}
	else if (peer.cmdElement.size() == 2)
	{
		if (_makeSourcePair(peer.cmdElement.peek(), peer.cmdElement.peek_next(), sourcePair))
			response = Directory::getTTL(sourcePair, ttl);
	}

	peer.writeBuffer += RESP[(short)response];
	if (response == Response::SUCCESS)
		peer.writeBuffer += " " + std::to_string(ttl);
}

void CmdInterpreter::_remove(Peer& peer)
{
	Response response = Response::BAD_PARAM;
	SourcePair sourcePair;

	if (peer.cmdElement.size() == 0)
		response = Directory::removeFromDir(peer.entry());
	else if (peer.cmdElement.size() == 1)
	{
		if (_isBase64(peer.cmdElement.peek()) && _makeSourcePair(peer.cmdElement.peek(), sourcePair))
			response = Directory::removeFromDir(sourcePair, peer.entry());
	}
	else if (peer.cmdElement.size() == 2)
	{
		if (_makeSourcePair(peer.cmdElement.peek(), peer.cmdElement.peek_next(), sourcePair))
			response = Directory::removeFromDir(sourcePair, peer.entry());
	}
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




	auto response = Directory::addToDir(peer.entry());
	peer.writeBuffer += RESP[(short)response] + " ";
	peer.entry()->printUID(peer.writeBuffer);
}

void CmdInterpreter::_search(Peer& peer)
{
}

void CmdInterpreter::_charge(Peer& peer)
{
	Response response = Response::BAD_PARAM;
	SourcePair sourcePair;
	short newTTL;
	
	if (peer.cmdElement.size() == 0)
		response = Directory::charge(peer.entry(), newTTL);
	else if (peer.cmdElement.size() == 1)
	{
		if (_isBase64(peer.cmdElement.peek()) && _makeSourcePair(peer.cmdElement.peek(), sourcePair))
			response = Directory::charge(sourcePair, peer.entry(), newTTL);
	}
	else if (peer.cmdElement.size() == 2)
	{
		if (_makeSourcePair(peer.cmdElement.peek(), peer.cmdElement.peek_next(), sourcePair))
			response = Directory::charge(sourcePair, peer.entry(), newTTL);
	}

	peer.writeBuffer += RESP[(short)response];
	if (response == Response::SUCCESS)
		peer.writeBuffer += " " + std::to_string(newTTL);
}