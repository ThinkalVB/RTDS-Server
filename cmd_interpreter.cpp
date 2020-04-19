#include "cmd_interpreter.h"
#include "cppcodec/base64_rfc4648.hpp"
#include "directory.h"
#include <regex>
#include "log.h"

#define V4_UID_MAX_CHAR 8
#define V6_UID_MAX_CHAR 24

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

bool CmdInterpreter::_isUID(const std::string_view& uid)
{
	if (uid.size() == V4_UID_MAX_CHAR || uid.size() == V6_UID_MAX_CHAR)
	{
		std::regex regx("[a-zA-Z0-9\+/]*");
		return std::regex_match(uid.cbegin(), uid.cend(), regx);
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
		else if (command == COMM[(short)Command::EXIT])
			s_exit(peer);
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
	}
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
	peer.peerEntry.Ev->printEntryCount(peer.writeBuffer);
}

void CmdInterpreter::s_exit(Peer& peer)
{
	peer.peerSocket->shutdown(asio::ip::tcp::socket::shutdown_both);
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
	std::string_view element;
	if (_extractElement(peer.receivedData, element))
	{
		if (_isUID(element))
		{
			if (element.size() == V4_UID_MAX_CHAR)
			{
				sourcePairV4 sourcePair;
				cppcodec::base64_rfc4648::decode(sourcePair.data(), sourcePair.size(), element);
				auto entry = Directory::findEntry(sourcePair);
				if (entry != nullptr)
					entry->printTTL(peer.writeBuffer);
				else 
					peer.writeBuffer += RESP[(short)Response::NO_EXIST];
			}
			else
			{
				sourcePairV6 sourcePair;
				cppcodec::base64_rfc4648::decode(sourcePair.data(), sourcePair.size(), element);
				auto entry = Directory::findEntry(sourcePair);
				if (entry != nullptr)
					entry->printTTL(peer.writeBuffer);
				else
					peer.writeBuffer += RESP[(short)Response::NO_EXIST];
			}
		}
	}
	else
	{
		if (peer.peerEntry.Ev->inDirectory())
		{
			peer.writeBuffer += RESP[(short)Response::SUCCESS] + " ";
			peer.writeBuffer += std::to_string((short)TTL::RESTRICTED_TTL);
		}
		else
			peer.writeBuffer += RESP[(short)Response::NO_EXIST];
	}
}

void CmdInterpreter::_remove(Peer& peer)
{

}

void CmdInterpreter::_flush(Peer& peer)
{
}

void CmdInterpreter::_update(Peer& peer)
{
}

void CmdInterpreter::_add(Peer& peer)
{
	auto response = Directory::addEntry(peer.peerEntry.Ev);
	peer.writeBuffer += RESP[(short)response] + " ";
	peer.peerEntry.Ev->printUID(peer.writeBuffer);
}

void CmdInterpreter::_search(Peer& peer)
{
	if (!peer.peerEntry.Ev->inDirectory())
		peer.writeBuffer += RESP[(short)Response::NO_EXIST];
	else
	{
		peer.writeBuffer += RESP[(short)Response::SUCCESS];
		peer.peerEntry.Ev->printExpand(peer.writeBuffer);
	}
}

void CmdInterpreter::_charge(Peer& peer)
{
	peer.writeBuffer += RESP[(short)Response::SUCCESS] + " ";
	peer.writeBuffer += std::to_string((short)TTL::RESTRICTED_TTL);
}