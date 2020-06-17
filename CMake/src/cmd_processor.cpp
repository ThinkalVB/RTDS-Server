#include "cmd_processor.h"
#include <regex>

const std::string CmdProcessor::RESP[] =
{
	"success",
	"bad_command",
	"bad_param",
	"wait_retry",
	"is_in_bg",
	"not_in_bg"
};

const std::string CmdProcessor::COMM[] =
{
	"broadcast",
	"ping",
	"listen",
	"leave",
	"exit"
};

void CmdProcessor::processCommand(Peer& peer)
{
	auto command = extractElement(peer.commandStr);
	if (command == COMM[(short)Command::PING])
		_cmd_ping(peer);
	else if (command == COMM[(short)Command::LISTEN])
		_cmd_listen(peer);
	else if (command == COMM[(short)Command::LEAVE])
		_cmd_leave(peer);
	else if (command == COMM[(short)Command::BROADCAST])
		_cmd_broadcast(peer);
	else if (command == COMM[(short)Command::EXIT])
		_cmd_exit(peer);
	else
		peer.respondWith(Response::BAD_COMMAND);
}

bool CmdProcessor::isBGID(const std::string_view& bgid)
{
	if (bgid.size() != 0 && isPrintable(bgid) && bgid.size() <= MAX_BGID_SIZE)
		return true;
	return false;
}

bool CmdProcessor::isTag(const std::string_view& tag)
{
	if (tag.size() != 0 && isPrintable(tag) && tag.size() <= MAX_TAG_SIZE)
		return true;
	return false;
}

bool CmdProcessor::isBmessage(const std::string_view& bMessage)
{
	if (bMessage.size() != 0 && isPrintable(bMessage) && bMessage.size() <= MAX_BROADCAST_SIZE)
		return true;
	return false;
}

bool CmdProcessor::isPrintable(const std::string_view& strElement)
{
	for (auto invChar : strElement)
	{
		if (!(invChar > 32 && invChar != 127))
			return false;
	}
	return true;
}

bool CmdProcessor::isPortNumber(const std::string portNStr, unsigned short& portNum)
{
	std::regex rgx("[0-9]{1,5}");
	if (std::regex_match(portNStr, rgx))
	{
		auto pNum = std::stoi(portNStr);
		if (pNum <= MAX_PORT_NUM_VALUE)
		{
			portNum = pNum;
			return true;
		}
	}
	return false;
}

bool CmdProcessor::isThreadCount(const std::string threadCStr, short& threadCount)
{
	std::regex rgx("[0-9]{1,5}");
	if (std::regex_match(threadCStr, rgx))
	{
		auto threadC = std::stoi(threadCStr);
		if (threadC <= MAX_THREAD_COUNT)
		{
			threadCount = threadC;
			return true;
		}
	}
	return false;
}

const std::string_view CmdProcessor::extractElement(std::string_view& command)
{
	auto endIndex = command.find_first_of(' ');
	if (endIndex == std::string::npos)
	{
		auto element = command;
		command.remove_prefix(command.size());
		return element;
	}
	else
	{
		auto element = command.substr(0, endIndex);
		if (command.size() == endIndex + 1)
			command.remove_prefix(endIndex);
		else
			command.remove_prefix(endIndex + 1);
		return element;
	}
}


void CmdProcessor::_cmd_ping(Peer& peer)
{
	if (peer.commandStr.empty())
		peer.printPingInfo();
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::_cmd_broadcast(Peer& peer)
{
	auto message = extractElement(peer.commandStr);
	auto bgTag = extractElement(peer.commandStr);
	if (isBmessage(message) && peer.commandStr.empty() )
	{
		if (bgTag.empty())
			peer.broadcast(message);
		else if (isTag(bgTag))
			peer.broadcast(message, bgTag);
		else
			peer.respondWith(Response::BAD_PARAM);
	}
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::_cmd_exit(Peer& peer)
{
	if (peer.commandStr.empty())
		peer.disconnect();
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::_cmd_listen(Peer& peer)
{
	auto bgID = extractElement(peer.commandStr);
	auto bgTag = extractElement(peer.commandStr);
	if (isTag(bgTag) && isBGID(bgID) && peer.commandStr.empty())
		peer.listenTo(bgID, bgTag);
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::_cmd_leave(Peer& peer)
{
	if (peer.commandStr.empty())
		peer.leaveBG();
	else
		peer.respondWith(Response::BAD_PARAM);
}
