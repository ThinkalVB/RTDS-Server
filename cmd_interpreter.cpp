#include "cmd_interpreter.h"
#include <charconv>
#include "directory.h"
#include <regex>

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
	"update",
	"remove",
	"mirror",
	"leave",
	"exit"
};

const char CmdInterpreter::PRI[] =
{
	'l',
	'p',
	'r',
	'~'
};

std::string CmdInterpreter::toPermission(const Permission& permission)
{
	std::string permStr;
	permStr += PRI[(short)permission.change];
	permStr += PRI[(short)permission.charge];
	permStr += PRI[(short)permission.remove];
	return permStr;
}

Permission CmdInterpreter::toPermission(const std::string_view& perm)
{
	Permission permission;
	permission.charge = toPrivilege(perm[0]);
	permission.change = toPrivilege(perm[1]);
	permission.remove = toPrivilege(perm[2]);
	return permission;
}

Privilege CmdInterpreter::toPrivilege(const char privilege)
{
	if (privilege == PRI[(short)Privilege::LIBERAL_ENTRY])
		return Privilege::LIBERAL_ENTRY;
	else if (privilege == PRI[(short)Privilege::PROTECTED_ENTRY])
		return Privilege::PROTECTED_ENTRY;
	else if (privilege == PRI[(short)Privilege::RESTRICTED_ENTRY])
		return Privilege::RESTRICTED_ENTRY;
	else
		return Privilege::ALL_ENTRY;
}

int CmdInterpreter::toIntiger(const std::string_view& intValue)
{
	int intiger;
	std::from_chars(intValue.data(), intValue.data() + intValue.size(), intiger);
	return intiger;
}

unsigned short CmdInterpreter::toInitialTTL(const Privilege maxPrivilege)
{
	if (maxPrivilege == Privilege::LIBERAL_ENTRY)
		return (unsigned short)TTL::LIBERAL_TTL;
	else if (maxPrivilege == Privilege::PROTECTED_ENTRY)
		return (unsigned short)TTL::PROTECTED_TTL;
	else
		return (unsigned short)TTL::RESTRICTED_TTL;
}

Permission CmdInterpreter::toDefPermission(const Privilege maxPriv)
{
	Permission permission;
	if (maxPriv == Privilege::RESTRICTED_ENTRY || maxPriv == Privilege::PROTECTED_ENTRY)
	{
		permission.charge = Privilege::PROTECTED_ENTRY;
		permission.change = Privilege::PROTECTED_ENTRY;
		permission.remove = Privilege::PROTECTED_ENTRY;
	}
	else
	{
		permission.charge = Privilege::LIBERAL_ENTRY;
		permission.change = Privilege::LIBERAL_ENTRY;
		permission.remove = Privilege::LIBERAL_ENTRY;
	}
	return permission;
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

bool CmdInterpreter::isPolicyPermission(const std::string_view& permission)
{
	std::regex regx("[~plr]{3}");
	return std::regex_match(permission.cbegin(), permission.cend(), regx);
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

bool CmdInterpreter::isValid(const Permission& perm, const Privilege maxPriv)
{
	if (maxPriv >= perm.change && maxPriv >= perm.remove && maxPriv >= perm.charge)
		return true;
	else
		return false;
}

bool CmdInterpreter::isComparable(const Permission& perm1, const Permission& perm2)
{
	if ((perm1.charge == perm2.charge || perm1.charge == Privilege::ALL_ENTRY || perm2.charge == Privilege::ALL_ENTRY) &&
		(perm1.change == perm2.change || perm1.change == Privilege::ALL_ENTRY || perm2.change == Privilege::ALL_ENTRY) &&
		(perm1.remove == perm2.remove || perm1.remove == Privilege::ALL_ENTRY || perm2.remove == Privilege::ALL_ENTRY))
		return true;
	else
		return false;
}


bool CmdInterpreter::makeCmdElement(ReceiveBuffer& dataBuffer, CommandElement& cmdElement, const std::size_t size)
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

bool CmdInterpreter::makeSourcePair(const std::string_view& uid, SPaddress& spAddress)
{
	if (isBase64(uid))
	{
		if (uid.size() == V4_UID_MAX_CHAR || uid.size() == V6_UID_MAX_CHAR)
		{
			spAddress = SPaddress(uid);
			return true;
		}
	}
	return false;
}

bool CmdInterpreter::makeSourcePair(const std::string_view& ipAddrStr, const std::string_view& portNum, SPaddress& spAddress)
{
	system::error_code ec;
	auto ipAddress = asio::ip::make_address(ipAddrStr, ec);
	unsigned short portNumber;
	if (ec == system::errc::success && isPortNumber(portNum, portNumber))
	{
		if (ipAddress.is_v4())
			spAddress = SPaddress(ipAddress.to_v4(), portNumber);
		else
			spAddress = SPaddress(ipAddress.to_v6(), portNumber);
		return true;
	}
	else
		return false;
}


const MutableData CmdInterpreter::tryExtractMutData(CommandElement& cmdElement)
{
	MutableData mutableData;
	if (cmdElement.size() >= 1 && isPermission(cmdElement.peek()))
	{
		auto permission = toPermission(cmdElement.pop_front());
		mutableData.setPermission(permission);
	}

	if (cmdElement.size() >= 1 && isDescription(cmdElement.peek()))
		mutableData.setDescription(cmdElement.pop_front());
	return mutableData;
}

const MutableData CmdInterpreter::tryExtractPolicyMD(CommandElement& cmdElement)
{
	MutableData mutableData;
	if (cmdElement.size() >= 1 && isPolicyPermission(cmdElement.peek()))
	{
		auto permission = toPermission(cmdElement.pop_front());
		mutableData.setPermission(permission);
	}

	if (cmdElement.size() >= 1 && isDescription(cmdElement.peek()))
		mutableData.setDescription(cmdElement.pop_front());
	return mutableData;
}

void CmdInterpreter::tryExtractSPA(CommandElement& cmdElement, SPaddress& spAddress)
{
	if (cmdElement.size() >= 2)
	{
		if (makeSourcePair(cmdElement.peek(), cmdElement.peek_next(), spAddress))
		{
			cmdElement.pop_front(2);
			return;
		}
	}
}

/* Function that process the commands from peer.....*/
void CmdInterpreter::processCommand(Peer& peer)
{
	auto command = peer.cmdElement.pop_front();
	if (command == COMM[(short)Command::PING] && peer.cmdElement.size() == 0)
		_ping(peer);
	else if (command == COMM[(short)Command::MIRROR])
		_mirror(peer);
	else if (command == COMM[(short)Command::LEAVE] && peer.cmdElement.size() == 0)
		_exit(peer);
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
	else if (command == COMM[(short)Command::UPDATE])
		_update(peer);
	else if (command == COMM[(short)Command::EXIT] && peer.cmdElement.size() == 0)
	{
		peer.terminatePeer();
		return;
	}
	else
		peer.writeBuffer = CmdInterpreter::RESP[(short)Response::BAD_COMMAND];
	peer.writeBuffer += '\x1e';				//!< Record separator
	peer.sendPeerData();
}

/* Commands without any arguments...................*/
void CmdInterpreter::_ping(Peer& peer)
{
	peer.writeBuffer += peer.spAddress.briefInfo();
}

void CmdInterpreter::_exit(Peer& peer)
{
	peer.removeFromMG();
	peer.writeBuffer += RESP[(short)Response::SUCCESS];
}

/* Commands with arguments..........................*/
void CmdInterpreter::_mirror(Peer& peer)
{
	auto policyMD = tryExtractPolicyMD(peer.cmdElement);
	if (peer.cmdElement.isEmpty() && policyMD.isValidPolicy())
	{
		peer.addToMG(policyMD);
		peer.writeBuffer += RESP[(short)Response::SUCCESS];
	}
	else
		peer.writeBuffer += RESP[(short)Response::BAD_PARAM];
}

void CmdInterpreter::_ttl(Peer& peer)
{
	auto targetSPA = peer.spAddress;
	tryExtractSPA(peer.cmdElement, targetSPA);

	if (!peer.cmdElement.isEmpty())
		peer.writeBuffer += RESP[(short)Response::BAD_PARAM];
	else
	{
		auto response = Directory::ttlEntry(targetSPA);
		if (response == Response::SUCCESS)
			response.printTTL(peer.writeBuffer);
		else
			response.printResponse(peer.writeBuffer);
	}
}

void CmdInterpreter::_remove(Peer& peer)
{
	auto targetSPA = peer.spAddress;
	tryExtractSPA(peer.cmdElement, targetSPA);

	if (!peer.cmdElement.isEmpty())
		peer.writeBuffer += RESP[(short)Response::BAD_PARAM];
	else
	{
		auto response = Directory::removeEntry(targetSPA, peer.spAddress);
		if (response == Response::SUCCESS)
		{
			auto notification = Notification::makeRemoveNote(targetSPA);
			peer.sendNoteToMG(notification, response.policy());
		}
		response.printResponse(peer.writeBuffer);
	}
}

void CmdInterpreter::_update(Peer& peer)
{
	auto targetSPA = peer.spAddress;
	tryExtractSPA(peer.cmdElement, targetSPA);
	auto mutData = tryExtractMutData(peer.cmdElement);

	if (!peer.cmdElement.isEmpty() || mutData.isEmpty())
		peer.writeBuffer += RESP[(short)Response::BAD_PARAM];
	else
	{
		auto response = Directory::updateEntry(targetSPA, peer.spAddress, mutData);
		if (response == Response::SUCCESS)
		{
			auto notification = Notification::makeAddNote(targetSPA);
			peer.sendNoteToMG(notification, response.policy());
		}
		response.printResponse(peer.writeBuffer);
	}
}

void CmdInterpreter::_add(Peer& peer)
{
	auto targetSPA = peer.spAddress;
	tryExtractSPA(peer.cmdElement, targetSPA);
	auto mutData = tryExtractMutData(peer.cmdElement);

	if (!peer.cmdElement.isEmpty())
		peer.writeBuffer += RESP[(short)Response::BAD_PARAM];
	else
	{
		auto response = Directory::createEntry(targetSPA, peer.spAddress, mutData);
		response.printResponse(peer.writeBuffer);
		if (response == Response::SUCCESS)
		{
			auto notification = Notification::makeAddNote(targetSPA);
			peer.sendNoteToMG(notification, response.policy());
			response.printTTL(peer.writeBuffer += " ");
		}
	}
}

void CmdInterpreter::_search(Peer& peer)
{
	auto targetSPA = peer.spAddress;
	tryExtractSPA(peer.cmdElement, targetSPA);
	if (peer.cmdElement.size() == 0)
	{
		auto response = Directory::searchEntry(targetSPA);
		if (response == Response::SUCCESS)
			response.printPolicy(peer.writeBuffer);
		else
			response.printResponse(peer.writeBuffer);
	}
	else if (peer.cmdElement.size() == 2)
	{
		auto policyMD = tryExtractPolicyMD(peer.cmdElement);
		if (policyMD.isValidPolicy())
			Directory::printEntryWith(peer.writeBuffer, policyMD);
	}
	else
		peer.writeBuffer += RESP[(short)Response::BAD_PARAM];
}

void CmdInterpreter::_charge(Peer& peer)
{
	auto targetSPA = peer.spAddress;
	tryExtractSPA(peer.cmdElement, targetSPA);

	if (!peer.cmdElement.isEmpty())
		peer.writeBuffer += RESP[(short)Response::BAD_PARAM];
	else
	{
		auto response = Directory::chargeEntry(targetSPA, peer.spAddress);
		if (response == Response::SUCCESS)
			response.printTTL(peer.writeBuffer);
		else
			response.printResponse(peer.writeBuffer);
	}
}