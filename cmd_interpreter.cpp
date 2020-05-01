#include "cmd_interpreter.h"
#include <charconv>
#include "directory.h"
#include "tockens.h"
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

	if (!cmdElement.isEmpty() && commandLine.empty())
	{
		cmdElement.reset_for_read();
		return true;
	}
	else
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

bool CmdInterpreter::extractSourcePair(CommandElement& cmdElement, SourcePair& sourcePair)
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

void CmdInterpreter::extractFlushCount(CommandElement& cmdElement, std::size_t& flushCount)
{
	if (cmdElement.size() >= 1 && isIntiger(cmdElement.peek()))
	{
		auto count = toIntiger(cmdElement.peek());
		if (count > 0)
		{
			flushCount = count;
			cmdElement.pop_front(1);
		}
	}
}

const MutableData CmdInterpreter::extractMutableData(CommandElement& cmdElement)
{
	MutableData mutableData;
	if (cmdElement.size() >= 1 && isPermission(cmdElement.peek()))
	{
		toPermission(cmdElement.peek(), mutableData.permission);
		cmdElement.pop_front(1);
		mutableData.havePermission = true;
	}

	if (cmdElement.size() >= 1 && isDescription(cmdElement.peek()))
	{
		mutableData.description = cmdElement.pop_front();
		mutableData.haveDescription = true;
	}
	return mutableData;
}

BaseEntry* CmdInterpreter::extractBaseEntry(BaseEntry* entry, CommandElement& cmdElement)
{
	SourcePair sourcePair;
	if (extractSourcePair(cmdElement, sourcePair))
		return Directory::findEntry(sourcePair);
	else
		return entry;
}


int CmdInterpreter::toIntiger(const std::string_view& intValue)
{
	int intiger;
	std::from_chars(intValue.data(), intValue.data() + intValue.size(), intiger);
	return intiger;
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
	if (isIntiger(portNumStr))
	{
		auto portNumber = toIntiger(portNumStr);
		if (portNumber <= MAX_PORT_NUM_VALUE)
		{
			portNum = (unsigned short)portNumber;
			return true;
		}
	}
	return false;
}

bool CmdInterpreter::isIntiger(const std::string_view& intValue)
{
	std::regex regx("[0-9]{1,8}");
	return std::regex_match(intValue.cbegin(), intValue.cend(), regx);
}

/* Function that process the commands from peer.....*/
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
	else
		peer.Buffer() = CmdInterpreter::RESP[(short)Response::BAD_COMMAND];
	peer.sendPeerData();
}

/* Commands without any arguments...................*/		
void CmdInterpreter::s_ping(BaseEntry* thisEntry, std::string& writeBuffer)
{
	Directory::printBrief(thisEntry, writeBuffer);
	writeBuffer += '\x1e';					//!< Record separator
}

void CmdInterpreter::s_count(std::string& writeBuffer)
{
	writeBuffer += std::to_string(Directory::getEntryCount());
	writeBuffer += '\x1e';					//!< Record separator
}

void CmdInterpreter::s_mirror(Peer& peer)
{
	peer.addToMirroringGroup();
	peer.Buffer() += RESP[(short)Response::SUCCESS];
	peer.Buffer() += '\x1e';				//!< Record separator
}

void CmdInterpreter::s_leave(Peer& peer)
{
	peer.removeFromMirroringGroup();
	peer.Buffer() += RESP[(short)Response::SUCCESS];
	peer.Buffer() += '\x1e';				//!< Record separator
}

/* Commands with arguments..........................*/
void CmdInterpreter::_ttl(BaseEntry* thisEntry, CommandElement& cmdElement, std::string& writeBuffer)
{
	auto entry = extractBaseEntry(thisEntry, cmdElement);

	if (!cmdElement.isEmpty())
		writeBuffer += RESP[(short)Response::BAD_PARAM] + '\x1e';
	else if (Directory::isInDirectory(entry))
	{
		auto ttl = Directory::getTTL(entry);
		writeBuffer += std::to_string(ttl) + '\x1e';
	}
	else
		writeBuffer += RESP[(short)Response::NO_EXIST] + '\x1e';
}

void CmdInterpreter::_remove(BaseEntry* thisEntry, CommandElement& cmdElement, std::string& writeBuffer)
{
	auto entry = extractBaseEntry(thisEntry, cmdElement);

	if (!cmdElement.isEmpty())
		writeBuffer += RESP[(short)Response::BAD_PARAM] + '\x1e';
	else if (Directory::isInDirectory(entry))
	{
		auto purgeTocken = Tocken::makePurgeTocken(entry, thisEntry);
		if (purgeTocken != nullptr)
		{
			Directory::removeEntry(purgeTocken);
			writeBuffer += RESP[(short)Response::SUCCESS] + '\x1e';
			Tocken::destroyTocken(purgeTocken);
		}
		else
			writeBuffer += RESP[(short)Response::NO_PRIVILAGE] + '\x1e';
	}
	else
		writeBuffer += RESP[(short)Response::NO_EXIST] + '\x1e';
}

void CmdInterpreter::_flush(CommandElement& cmdElement, std::string& writeBuffer)
{
	std::size_t flushCount = SIZE_MAX;
	extractFlushCount(cmdElement, flushCount);

	if (!cmdElement.isEmpty())
		writeBuffer += RESP[(short)Response::BAD_PARAM] + '\x1e';
	else
		Directory::flushEntries(writeBuffer, flushCount);
}

void CmdInterpreter::_update(BaseEntry* thisEntry, CommandElement& cmdElement, std::string& writeBuffer)
{
	auto entry = extractBaseEntry(thisEntry, cmdElement);
	auto mutableData = extractMutableData(cmdElement);

	if (!cmdElement.isEmpty() || mutableData.isEmpty())
		writeBuffer += RESP[(short)Response::BAD_PARAM] + '\x1e';
	else if (Directory::isInDirectory(entry))
	{
		auto updateTocken = Tocken::makeUpdateTocken(entry, thisEntry);
		if (updateTocken != nullptr)
		{
			auto response = Directory::updateEntry(updateTocken, mutableData);
			Tocken::destroyTocken(updateTocken);
			writeBuffer += RESP[(short)response] + '\x1e';
		}
		else
			writeBuffer += RESP[(short)Response::NO_PRIVILAGE] + '\x1e';
	}
	else
		writeBuffer += RESP[(short)Response::NO_EXIST] + '\x1e';
}

void CmdInterpreter::_add(BaseEntry* thisEntry, CommandElement& cmdElement, std::string& writeBuffer)
{
	SourcePair sourcePair;
	BaseEntry* entry;

	if (extractSourcePair(cmdElement, sourcePair))
		entry = Directory::makeEntry(sourcePair);
	else
		entry = thisEntry;
	auto mutableData = extractMutableData(cmdElement);

	if (!cmdElement.isEmpty())
	{
		writeBuffer += RESP[(short)Response::BAD_PARAM] + '\x1e';
		return;
	}
	else if (Directory::isInDirectory(entry))
		writeBuffer += RESP[(short)Response::REDUDANT_DATA] + " ";
	else
	{
		auto insertionTocken = Tocken::makeInsertionTocken(entry, thisEntry);
		auto response = Directory::insertEntry(insertionTocken, mutableData);
		Tocken::destroyTocken(insertionTocken);
		if (response == Response::SUCCESS)
			writeBuffer += RESP[(short)Response::SUCCESS] + " ";
		else
		{
			writeBuffer += RESP[(short)response] + '\x1e';
			return;
		}
	}
	Directory::printUID(entry, writeBuffer);
	writeBuffer += '\x1e';
}

void CmdInterpreter::_search(BaseEntry* thisEntry, CommandElement& cmdElement, std::string& writeBuffer)
{
}

void CmdInterpreter::_charge(BaseEntry* thisEntry, CommandElement& cmdElement, std::string& writeBuffer)
{
	auto entry = extractBaseEntry(thisEntry, cmdElement);

	if (!cmdElement.isEmpty())
		writeBuffer += RESP[(short)Response::BAD_PARAM] + '\x1e';
	else if (Directory::isInDirectory(entry))
	{
		auto chargeTocken = Tocken::makeChargeTocken(entry, thisEntry);
		if (chargeTocken != nullptr)
		{
			auto newTTL = Directory::chargeEntry(chargeTocken);
			writeBuffer += std::to_string(newTTL) + '\x1e';
			Tocken::destroyTocken(chargeTocken);
		}
		else
			writeBuffer += RESP[(short)Response::NO_PRIVILAGE ] + '\x1e';
	}
	else
		writeBuffer += RESP[(short)Response::NO_EXIST] + '\x1e';
}
