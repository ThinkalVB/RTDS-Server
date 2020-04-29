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
	"success",
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

bool CmdInterpreter::makeCmdElement(std::array<char, RTDS_BUFF_SIZE>& dataBuffer, CommandElement& cmdElement, std::size_t size)
{
	std::string_view commandLine;
	if (dataBuffer[size - 1] == ';')
	{
		dataBuffer[size - 1] = '\0';
		commandLine = (char*)dataBuffer.data();
		cmdElement.reset();
	}
	else
		return false;

	while (!commandLine.empty())
	{
		if (cmdElement.size() == 5)
			return false;

		auto cmdSize = commandLine.size();
		auto endIndex = commandLine.find_first_not_of(' ');
		commandLine.remove_prefix(std::min(endIndex, cmdSize));

		cmdSize = commandLine.size();
		if (cmdSize > 0)
		{
			if (commandLine[0] == '[')
			{
				endIndex = commandLine.find_last_not_of(' ');
				if (endIndex == std::string::npos)
					cmdElement.push_back(commandLine.substr(0, commandLine.size()));
				else
					cmdElement.push_back(commandLine.substr(0, endIndex + 1));
				commandLine.remove_prefix(commandLine.size());
			}
			else
			{
				endIndex = commandLine.find(' ');
				if (endIndex == std::string::npos)
				{
					cmdElement.push_back(commandLine);
					commandLine.remove_prefix(commandLine.size());
					break;
				}
				else
				{
					cmdElement.push_back(commandLine.substr(0, endIndex));
					commandLine.remove_prefix(endIndex);
				}
			}
		}
		else
			break;
	}

	if (cmdElement.size() > 0 && commandLine.empty())
	{
		cmdElement.reset_for_read();
		return true;
	}
	else
		return false;
}

bool CmdInterpreter::extractFlushParam(CommandElement& cmdElement, std::size_t& flushCount)
{
	auto flushPara = cmdElement.peek();
	std::regex regx("[0-9]{1,5}");
	if (std::regex_match(flushPara.cbegin(), flushPara.cend(), regx))
	{
		std::from_chars(flushPara.data(), flushPara.data() + flushPara.size(), flushCount);
		if (flushCount != 0)
		{
			cmdElement.pop_front(1);
			return true;
		}
	}
	return false;
}

Privilege CmdInterpreter::toPrivilege(const char& privilege)
{
	if (privilege == 'l')
		return Privilege::LIBERAL_ENTRY;
	else if (privilege == 'p')
		return Privilege::PROTECTED_ENTRY;
	else
		return Privilege::RESTRICTED_ENTRY;
}

void CmdInterpreter::toPermission(const std::string_view& perm, Permission& permission)
{
	permission.charge = toPrivilege(perm[0]);
	permission.change = toPrivilege(perm[1]);
	permission.remove = toPrivilege(perm[2]);
}

TTL CmdInterpreter::toTTL(Privilege maxPrivilege)
{
	if (maxPrivilege == Privilege::LIBERAL_ENTRY)
		return TTL::LIBERAL_TTL;
	else if (maxPrivilege == Privilege::PROTECTED_ENTRY)
		return TTL::PROTECTED_TTL;
	else
		return TTL::RESTRICTED_TTL;
}

bool CmdInterpreter::isBase64(const std::string_view& uid)
{
	std::regex regx("[a-zA-Z0-9\+/]*");
	return std::regex_match(uid.cbegin(), uid.cend(), regx);
}

bool CmdInterpreter::isDescription(const std::string_view& desc)
{
	if (desc[0] == '[' && desc[desc.size() - 1] == ']' && desc.size() <= MAX_DESC_SIZE)
		return true;
	return false;
}

bool CmdInterpreter::isPermission(const std::string_view& permission)
{
	std::regex regx("[plr]{3}");
	return std::regex_match(permission.cbegin(), permission.cend(), regx);
}

bool CmdInterpreter::isPortNumber(const std::string_view& portNumStr, unsigned short& portNum)
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
	}
	return false;
}

bool CmdInterpreter::makeSourcePair(const std::string_view& ipAddrStr, const std::string_view& portNum, SourcePair& sourcePair)
{
	system::error_code ec;
	auto ipAddress = asio::ip::make_address(ipAddrStr, ec);
	unsigned short portNumber;
	if (ec == system::errc::success && isPortNumber(portNum, portNumber))
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

bool CmdInterpreter::makeSourcePair(const std::string_view& uid, SourcePair& sourcePair)
{
	if (isBase64(uid))
	{
		if (uid.size() == V4_UID_MAX_CHAR)
		{
			cppcodec::base64_rfc4648::decode(sourcePair.SP.V4.data(), sourcePair.SP.V4.size(), uid);
			sourcePair.version = Version::V4;
			return true;
		}
		else if (uid.size() == V6_UID_MAX_CHAR)
		{
			cppcodec::base64_rfc4648::decode(sourcePair.SP.V6.data(), sourcePair.SP.V6.size(), uid);
			sourcePair.version = Version::V6;
			return true;
		}
	}
	return false;
}

bool CmdInterpreter::makeSourcePair(CommandElement& cmdElement, SourcePair& sourcePair)
{
	if (cmdElement.size() >= 1)
	{
		if (makeSourcePair(cmdElement.peek(), sourcePair))
		{
			cmdElement.pop_front(1);
			return true;
		}
	}
	
	if (cmdElement.size() >= 2)
	{
		if (makeSourcePair(cmdElement.peek(), cmdElement.peek_next(), sourcePair))
		{
			cmdElement.pop_front(2);
			return true;
		}
	}
	return false;
}

Response CmdInterpreter::_updateLockedEntry(UpdateTocken& updateTocken, CommandElement& cmdElement)
{
	auto response = Response::BAD_PARAM;
	if (cmdElement.size() == 2)
	{
		if (isPermission(cmdElement.peek()) && isDescription(cmdElement.peek_next()))
		{
			Permission permission;
			toPermission(cmdElement.peek(), permission);

			response = Directory::update(updateTocken, permission);
			if (response == Response::SUCCESS)
				Directory::update(updateTocken, cmdElement.peek_next());
		}
	}

	if (cmdElement.size() == 1)
	{
		if (isPermission(cmdElement.peek()))
		{
			Permission permission;
			toPermission(cmdElement.peek(), permission);
			response = Directory::update(updateTocken, permission);
		}	
		else if (isDescription(cmdElement.peek()))
		{
			Directory::update(updateTocken, cmdElement.peek());
			response = Response::SUCCESS;
		}
	}
	return response;
}

void CmdInterpreter::processCommand(Peer& peer)
{
	auto command = peer.cmdElement().pop_front();
	if (command == COMM[(short)Command::PING] && peer.cmdElement().size() == 0)
		s_ping(peer.entry(), peer.Buffer());
	else if (command == COMM[(short)Command::COUNT] && peer.cmdElement().size() == 0)
		s_count(peer.Buffer());
	else if (command == COMM[(short)Command::MIRROR] && peer.cmdElement().size() == 0)
		s_mirror(peer);
	else if (command == COMM[(short)Command::LEAVE] && peer.cmdElement().size() == 0)
		s_leave(peer);
	else if (command == COMM[(short)Command::ADD])
		_add(peer.entry(), peer.cmdElement(), peer.Buffer());
	else if (command == COMM[(short)Command::SEARCH])
		_search(peer.entry(), peer.cmdElement(), peer.Buffer());
	else if (command == COMM[(short)Command::CHARGE])
		_charge(peer.entry(), peer.cmdElement(), peer.Buffer());
	else if (command == COMM[(short)Command::TTL])
		_ttl(peer.entry(), peer.cmdElement(), peer.Buffer());
	else if (command == COMM[(short)Command::REMOVE])
		_remove(peer.entry(), peer.cmdElement(), peer.Buffer());
	else if (command == COMM[(short)Command::FLUSH])
		_flush(peer.cmdElement(), peer.Buffer());
	else if (command == COMM[(short)Command::UPDATE])
		_update(peer.entry(), peer.cmdElement(), peer.Buffer());
	else if (command == COMM[(short)Command::EXIT] && peer.cmdElement().size() == 0)
	{
		peer.terminatePeer();
		return;
	}

	if (peer.Buffer().size() == 0)
		peer.Buffer() = CmdInterpreter::RESP[(short)Response::BAD_COMMAND];
	peer.sendPeerData();
}

/* Commands without any arguments				*/		
void CmdInterpreter::s_ping(BaseEntry* entry, std::string& writeBuffer)
{
	writeBuffer += RESP[(short)Response::SUCCESS] + " ";
	Directory::printBrief(entry, writeBuffer);
}

void CmdInterpreter::s_count(std::string& writeBuffer)
{
	writeBuffer += RESP[(short)Response::SUCCESS] + " ";
	writeBuffer += std::to_string(Directory::getEntryCount());
}

void CmdInterpreter::s_mirror(Peer& peer)
{
	peer.Buffer() += RESP[(short)Response::SUCCESS];
	peer.addToMirroringGroup();
}

void CmdInterpreter::s_leave(Peer& peer)
{
	peer.Buffer() += RESP[(short)Response::SUCCESS];
	peer.removeFromMirroringGroup();
}

/* Commands with arguments						*/
void CmdInterpreter::_ttl(BaseEntry* entry, CommandElement& cmdElement, std::string& writeBuffer)
{
	Response response = Response::BAD_PARAM;
	SourcePair sourcePair;
	short ttl;

	if (cmdElement.size() == 0)
		response = Directory::getTTL(entry, ttl);
	else if (makeSourcePair(cmdElement, sourcePair) && cmdElement.size() == 0)
		response = Directory::getTTL(sourcePair, ttl);

	writeBuffer += RESP[(short)response];
	if (response == Response::SUCCESS)
		writeBuffer += " " + std::to_string(ttl);
}

void CmdInterpreter::_remove(BaseEntry* entry, CommandElement& cmdElement, std::string& writeBuffer)
{
	Response response = Response::BAD_PARAM;
	SourcePair sourcePair;

	if (cmdElement.size() == 0)
		response = Directory::removeFromDir(entry);
	else if (makeSourcePair(cmdElement, sourcePair) && cmdElement.size() == 0)
		response = Directory::removeFromDir(sourcePair, entry);

	writeBuffer += RESP[(short)response];
}

void CmdInterpreter::_flush(CommandElement& cmdElement, std::string& writeBuffer)
{
	std::size_t flushCount;
	Response response = Response::BAD_PARAM;
	if (cmdElement.size() == 1 && extractFlushParam(cmdElement, flushCount))
		response = Directory::flushEntries(writeBuffer, flushCount);
	else if (cmdElement.size() == 0)
		response = Directory::flushEntries(writeBuffer);

	if (response != Response::SUCCESS)
		writeBuffer += RESP[(short)response];
}

void CmdInterpreter::_update(BaseEntry* entry, CommandElement& cmdElement, std::string& writeBuffer)
{
	Response response = Response::BAD_PARAM;
	SourcePair sourcePair;
	UpdateTocken updateTocken;

	if (makeSourcePair(cmdElement, sourcePair))
		response = Directory::getUpdateTocken(sourcePair, entry, updateTocken);
	else
		response = Directory::getUpdateTocken(entry, updateTocken);

	if (response == Response::SUCCESS)
	{
		response = _updateLockedEntry(updateTocken, cmdElement);
		Directory::releaseUpdateTocken(updateTocken);
	}
	writeBuffer += RESP[(short)response];
}

void CmdInterpreter::_add(BaseEntry* entry, CommandElement& cmdElement, std::string& writeBuffer)
{
	Response response = Response::BAD_PARAM;
	SourcePair sourcePair;
	InsertionTocken insertionTocken;

	if (makeSourcePair(cmdElement, sourcePair))
		response = Directory::addToDir(sourcePair, entry, insertionTocken);
	else 
		response = Directory::addToDir(entry, insertionTocken);

	if (response == Response::SUCCESS)
	{
		if (cmdElement.size() > 0)
			response = _updateLockedEntry(insertionTocken, cmdElement);
		Directory::releaseInsertionTocken(insertionTocken, response);
	}

	writeBuffer += RESP[(short)response];
	if (response == Response::SUCCESS || response == Response::REDUDANT_DATA)
	{
		writeBuffer += " ";
		Directory::printUID(insertionTocken.entry(), writeBuffer);
	}
}

void CmdInterpreter::_search(BaseEntry* entry, CommandElement& cmdElement, std::string& writeBuffer)
{
	Response response = Response::BAD_PARAM;
	SourcePair sourcePair;
	BaseEntry* foundEntry = nullptr;

	if (makeSourcePair(cmdElement, sourcePair) && cmdElement.size() == 0)
		response = Directory::searchEntry(sourcePair, foundEntry);
	else if (cmdElement.size() == 0)
	{
		foundEntry = entry;
		response = Response::SUCCESS;
	}

	if (response == Response::SUCCESS)
		Directory::printExpand(foundEntry, writeBuffer);
	else
		writeBuffer += RESP[(short)response];
}

void CmdInterpreter::_charge(BaseEntry* entry, CommandElement& cmdElement, std::string& writeBuffer)
{
	Response response = Response::BAD_PARAM;
	SourcePair sourcePair;
	short newTTL;
	
	if (cmdElement.size() == 0)
		response = Directory::charge(entry, newTTL);
	else if (makeSourcePair(cmdElement, sourcePair) && cmdElement.size() == 0)
		response = Directory::charge(sourcePair, entry, newTTL);

	writeBuffer += RESP[(short)response];
	if (response == Response::SUCCESS)
		writeBuffer += " " + std::to_string(newTTL);
}