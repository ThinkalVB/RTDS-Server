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
	"ping",
	"listen",
	"hear",
	"leave",
	"exit",
	"login",
	"abort",
	"status"
};


bool CmdProcessor::isBGID(const std::string_view& bgid)
{
	if (bgid.size() >= MIN_BGID_SIZE && isConsistent(bgid) && bgid.size() <= MAX_BGID_SIZE)
		return true;
	return false;
}

bool CmdProcessor::isTag(const std::string_view& tag)
{
	if (tag.size() >= MIN_BGID_SIZE && isConsistent(tag) && tag.size() <= MAX_TAG_SIZE)
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


void CmdProcessor::processCommand(TCPpeer& peer)
{
	auto commandStr = peer.getCommandString();
	auto command = extractElement(commandStr);
	if (command == COMM[(short)Command::PING])
		mTCP_ping(peer, commandStr);
	else if (command == COMM[(short)Command::LISTEN])
		mTCP_listen(peer, commandStr);
	else if (command == COMM[(short)Command::HEAR])
		mTCP_hear(peer, commandStr);
	else if (command == COMM[(short)Command::LEAVE])
		mTCP_leave(peer, commandStr);
	else if (command == COMM[(short)Command::BROADCAST])
		mTCP_broadcast(peer, commandStr);
	else if (command == COMM[(short)Command::EXIT])
		mTCP_exit(peer, commandStr);
	else
		peer.respondWith(Response::BAD_COMMAND);
}

void CmdProcessor::mTCP_ping(TCPpeer& peer, std::string_view& commandStr)
{
	if (commandStr.empty())
		peer.printPingInfo();
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mTCP_broadcast(TCPpeer& peer, std::string_view& commandStr)
{
	auto message = extractElement(commandStr);
	auto bgTag = extractElement(commandStr);
	if (isBmessage(message) && commandStr.empty())
	{
		if (bgTag == ALL_TAG)
			peer.broadcast(message);
		else if (isTag(bgTag))
			peer.broadcast(message, bgTag);
		else
			peer.respondWith(Response::BAD_PARAM);
	}
	else
		peer.respondWith(Response::BAD_PARAM);

}

void CmdProcessor::mTCP_exit(TCPpeer& peer, std::string_view& commandStr)
{
	if (commandStr.empty())
		peer.disconnect();
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mTCP_listen(TCPpeer& peer, std::string_view& commandStr)
{
	auto bgID = extractElement(commandStr);
	auto bgTag = extractElement(commandStr);
	if (isTag(bgTag) && isBGID(bgID) && commandStr.empty())
		peer.listenTo(bgID, bgTag);
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mTCP_hear(TCPpeer& peer, std::string_view& commandStr)
{
	auto bgID = extractElement(commandStr);
	auto bgTag = extractElement(commandStr);
	if (isTag(bgTag) && isBGID(bgID) && commandStr.empty())
		peer.hearTo(bgID, bgTag);
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mTCP_leave(TCPpeer& peer, std::string_view& commandStr)
{
	if (commandStr.empty())
		peer.leaveBG();
	else
		peer.respondWith(Response::BAD_PARAM);
}


void CmdProcessor::processCommand(UDPpeer& peer, AdancedBuffer& dataBuffer)
{
	auto commandStr = dataBuffer.getStringView();
	auto command = extractElement(commandStr);
	if (command == COMM[(short)Command::PING])
		mUDP_ping(peer, commandStr, dataBuffer);
	else if (command == COMM[(short)Command::BROADCAST])
		mUDP_broadcast(peer, commandStr, dataBuffer);
	else if (command == COMM[(short)Command::LISTEN])
		peer.respondWith(Response::NOT_ALLOWED, dataBuffer);
	else if (command == COMM[(short)Command::HEAR])
		peer.respondWith(Response::NOT_ALLOWED, dataBuffer);
	else if (command == COMM[(short)Command::LEAVE])
		peer.respondWith(Response::NOT_ALLOWED, dataBuffer);
	else if (command == COMM[(short)Command::EXIT])
		peer.respondWith(Response::NOT_ALLOWED, dataBuffer);
	else
		peer.respondWith(Response::BAD_COMMAND, dataBuffer);
}

void CmdProcessor::mUDP_ping(UDPpeer& peer, std::string_view& commandStr, AdancedBuffer& dataBuffer)
{
	if (commandStr.empty())
		peer.printPingInfo(dataBuffer);
	else
		peer.respondWith(Response::BAD_PARAM, dataBuffer);
}

void CmdProcessor::mUDP_broadcast(UDPpeer& peer, std::string_view& commandStr, AdancedBuffer& dataBuffer)
{
	auto message = extractElement(commandStr);
	auto bgTag = extractElement(commandStr);
	auto bgID = extractElement(commandStr);

	if (isBmessage(message) && commandStr.empty())
	{
		if (bgTag == "*" && isBGID(bgID))
			peer.broadcast(message, bgID, dataBuffer);
		else if (isTag(bgTag) && isBGID(bgID))
			peer.broadcast(message, bgID, bgTag, dataBuffer);
		else
			peer.respondWith(Response::BAD_PARAM, dataBuffer);
	}
	else
		peer.respondWith(Response::BAD_PARAM, dataBuffer);
}


void CmdProcessor::processCommand(SSLpeer& peer)
{
	auto commandStr = peer.getCommandString();
	auto command = extractElement(commandStr);
	if (command == COMM[(short)Command::LOGIN])
		mSSL_login(peer, commandStr);
	else if (command == COMM[(short)Command::STATUS])
		mSSL_status(peer, commandStr);
	else if (command == COMM[(short)Command::EXIT])
		mSSL_exit(peer, commandStr);
	else if (command == COMM[(short)Command::ABORT])
		mSSL_abort(peer, commandStr);
	else
		peer.respondWith(Response::BAD_COMMAND);
}

void CmdProcessor::mSSL_login(SSLpeer& peer, std::string_view& commandStr)
{
	auto usrName = extractElement(commandStr);
	auto password = extractElement(commandStr);
	if (isUsername(usrName) && isPassword(password) && commandStr.empty())
		peer.login(usrName, password);
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mSSL_exit(SSLpeer& peer, std::string_view& commandStr)
{
	if (commandStr.empty())
		peer.disconnect();
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mSSL_status(SSLpeer& peer, std::string_view& commandStr)
{
	if (commandStr.empty())
		peer.status();
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mSSL_abort(SSLpeer& peer, std::string_view& commandStr)
{
	if (commandStr.empty())
		peer.abort();
	else
		peer.respondWith(Response::BAD_PARAM);
}
