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
		mTCP_ping(peer);
	else if (command == COMM[(short)Command::LISTEN])
		mTCP_listen(peer);
	else if (command == COMM[(short)Command::LEAVE])
		mTCP_leave(peer);
	else if (command == COMM[(short)Command::BROADCAST])
		mTCP_broadcast(peer);
	else if (command == COMM[(short)Command::EXIT])
		mTCP_exit(peer);
	else
		peer.respondWith(Response::BAD_COMMAND);
}

void CmdProcessor::processCommand(AdancedBuffer& dataBuffer, const asio::ip::udp::endpoint udpEp)
{
	auto cmdStr = dataBuffer.getStringView();
	std::string response = "[R] ";

	auto command = extractElement(cmdStr);
	if (command == COMM[(short)Command::PING])
		response += toSAPInfo(udpEp);
	else
		response += CmdProcessor::RESP[(short)Response::BAD_COMMAND];
	dataBuffer = response;
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

std::string CmdProcessor::toSAPInfo(const asio::ip::udp::endpoint udpEp)
{
	std::string saPairStr;
	auto m_ipAddr = udpEp.address();
	auto m_portNumber = udpEp.port();

	if (m_ipAddr.is_v4())
	{
		saPairStr += STR_V4;
		auto _ipAddr4 = m_ipAddr.to_v4();
		saPairStr += " " + _ipAddr4.to_string();
	}
	else
	{
		saPairStr += STR_V6;
		auto _ipAddr6 = m_ipAddr.to_v6();
		if (_ipAddr6.is_v4_mapped())
			saPairStr += " " + _ipAddr6.to_v4().to_string();
		else
			saPairStr += " " + _ipAddr6.to_string();
	}
	saPairStr += " " + std::to_string(m_portNumber);
	return saPairStr;
}


void CmdProcessor::mTCP_ping(Peer& peer)
{
	if (peer.commandStr.empty())
		peer.printPingInfo();
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mTCP_broadcast(Peer& peer)
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

void CmdProcessor::mTCP_exit(Peer& peer)
{
	if (peer.commandStr.empty())
		peer.disconnect();
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mTCP_listen(Peer& peer)
{
	auto bgID = extractElement(peer.commandStr);
	auto bgTag = extractElement(peer.commandStr);
	if (isTag(bgTag) && isBGID(bgID) && peer.commandStr.empty())
		peer.listenTo(bgID, bgTag);
	else
		peer.respondWith(Response::BAD_PARAM);
}

void CmdProcessor::mTCP_leave(Peer& peer)
{
	if (peer.commandStr.empty())
		peer.leaveBG();
	else
		peer.respondWith(Response::BAD_PARAM);
}
