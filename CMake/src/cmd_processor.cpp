#include "cmd_processor.h"
#include <regex>

const std::string CmdProcessor::RESP[] =
{
	"success",
	"bad_command",
	"bad_param",
	"wait_retry",
	"is_in_bg",
	"not_in_bg",
	"not_allowed"
};

const std::string CmdProcessor::COMM[] =
{
	"broadcast",
	"message",
	"ping",
	"listen",
	"leave",
	"change",
	"exit",
	"login",
	"abort",
	"status"
};


TagType CmdProcessor::getTagType(const std::string_view& tag)
{
	if (isTag(tag))
	{
		if (tag == ALL_TAG)
			return TagType::ALL;
		else if (tag == OWN_TAG)
			return TagType::OWN;
		else if (tag.empty())
			return TagType::EMPTY;
		else if (tag.size() >= MIN_TAG_SIZE)
			return TagType::GENERAL;
		else
			return TagType::ERR;
	}
	else
		return TagType::ERR;
}

bool CmdProcessor::isTag(const std::string_view& tag)
{
	if (isConsistent(tag) && tag.size() <= MAX_TAG_SIZE)
		return true;
	return false;
}

bool CmdProcessor::isGeneralTag(const std::string_view& tag)
{
	if (tag.size() >= MIN_TAG_SIZE && isConsistent(tag) && tag.size() <= MAX_TAG_SIZE)
		return true;
	return false;
}

bool CmdProcessor::isBGID(const std::string_view& bgid)
{
	if (bgid.size() >= MIN_BGID_SIZE && isConsistent(bgid) && bgid.size() <= MAX_BGID_SIZE)
		return true;
	return false;
}

bool CmdProcessor::isBmessage(const std::string_view& bMessage)
{
	if (bMessage.size() != 0 && isPrintable(bMessage) && bMessage.size() <= MAX_BROADCAST_SIZE)
		return true;
	return false;
}

bool CmdProcessor::isConsistent(const std::string_view& strElement)
{
	for (auto invChar : strElement)
	{
		if (!(invChar > 32 && invChar != 127))
			return false;
	}
	return true;
}

bool CmdProcessor::isPrintable(const std::string_view& strElement)
{
	for (auto invChar : strElement)
	{
		if (!(invChar >= 32 && invChar != 127))
			return false;
	}
	return true;
}

bool CmdProcessor::isUsername(const std::string_view& username)
{
	if (username.size() == 0 || username.size() > USRN_MAX_SIZE)
		return false;
	for (auto invChar : username)
	{
		if (!(invChar > 32 && invChar != 127))
			return false;
	}
	return true;
}

bool CmdProcessor::isPassword(const std::string_view& password)
{
	if (password.size() != 0 && isPrintable(password) && password.size() <= PASS_MAX_SIZE)
		return true;
	return false;
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
		if (threadC <= MAX_THREAD_COUNT && threadC >= MIN_THREAD_COUNT)
		{
			threadCount = threadC;
			return true;
		}
	}
	return false;
}

const std::string_view CmdProcessor::extractElement(std::string_view& command)
{
	auto endIndex = command.find_first_of('\t');
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


void CmdProcessor::processCommand(UDPpeer& peer)
{
	auto commandStr = peer.getCommandString();
	auto command = extractElement(commandStr);
	
	if (command == COMM[(short)Command::BROADCAST])
		mUDP_broadcast(peer, commandStr);
	else if (command == COMM[(short)Command::MESSAGE])
		mUDP_message(peer, commandStr);
	else if (command == COMM[(short)Command::PING])
		mUDP_ping(peer, commandStr);
	else if (command == COMM[(short)Command::LISTEN])
		peer.respondWith(Response::NOT_ALLOWED);
	else if (command == COMM[(short)Command::LEAVE])
		peer.respondWith(Response::NOT_ALLOWED);
	else if (command == COMM[(short)Command::CHANGE])
		peer.respondWith(Response::NOT_ALLOWED);
	else if (command == COMM[(short)Command::EXIT])
		peer.respondWith(Response::NOT_ALLOWED);
	else
		peer.respondWith(Response::BAD_COMMAND);
}

void CmdProcessor::mUDP_ping(UDPpeer& peer, std::string_view& commandStr)
{
	if (commandStr.empty())
		peer.printPingInfo();
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mUDP_broadcast(UDPpeer& peer, std::string_view& commandStr)
{
	auto message = extractElement(commandStr);
	auto bgID = extractElement(commandStr);
	auto bgTag = extractElement(commandStr);

	if (isBmessage(message) && commandStr.empty() && isBGID(bgID))
		peer.messageTo(message, bgID, bgTag);
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mUDP_message(UDPpeer& peer, std::string_view& commandStr)
{
	auto message = extractElement(commandStr);
	auto bgID = extractElement(commandStr);
	auto bgTag = extractElement(commandStr);

	if (isBmessage(message) && commandStr.empty() && isBGID(bgID))
		peer.messageTo(message, bgID, bgTag);
	else
		peer.respondWith(Response::BAD_PARAM);
}


void CmdProcessor::processCommand(SSLccm& peer)
{
	auto commandStr = peer.getCommandString();
	auto command = extractElement(commandStr);
	if (command == COMM[(short)Command::LOGIN])
		mCCM_login(peer, commandStr);
	else if (command == COMM[(short)Command::STATUS])
		mCCM_status(peer, commandStr);
	else if (command == COMM[(short)Command::EXIT])
		mCCM_exit(peer, commandStr);
	else if (command == COMM[(short)Command::ABORT])
		mCCM_abort(peer, commandStr);
	else
		peer.respondWith(Response::BAD_COMMAND);
}

void CmdProcessor::mCCM_login(SSLccm& peer, std::string_view& commandStr)
{
	auto usrName = extractElement(commandStr);
	auto password = extractElement(commandStr);
	if (isUsername(usrName) && isPassword(password) && commandStr.empty())
		peer.login(usrName, password);
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mCCM_exit(SSLccm& peer, std::string_view& commandStr)
{
	if (commandStr.empty())
		peer.disconnect();
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mCCM_status(SSLccm& peer, std::string_view& commandStr)
{
	if (commandStr.empty())
		peer.status();
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mCCM_abort(SSLccm& peer, std::string_view& commandStr)
{
	if (commandStr.empty())
		peer.abort();
	else
		peer.respondWith(Response::BAD_PARAM);
}