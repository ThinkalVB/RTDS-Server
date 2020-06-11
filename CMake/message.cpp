#include "message.h"
#include "common.h"
#include "log.h"

std::queue<Message*> Message::_messageQ;
std::mutex Message::_QinsLock;
std::mutex Message::_QremLock;

Message::Message(const std::string& messageStr)
{
	LOG(Log::log(messageStr);)
	_createdTime = std::chrono::system_clock::now();
	messageBuf = messageStr;
}

bool Message::haveExpired() const
{
	TimePoint timeNow = std::chrono::system_clock::now();
	auto diffTime = std::chrono::duration_cast<std::chrono::minutes>(timeNow - _createdTime);
	auto timePassed = diffTime.count();
	if (timePassed > MIN_MSG_KEEP_TIME)
		return true;
	else
		return false;
}

const Message* Message::makeAddMsg(const SApair& saPair)
{
	auto addMssg = "[+] " + saPair.toString();
	Message* message = nullptr;
	try {
		message = new Message(addMssg);
		std::lock_guard<std::mutex> lock(_QinsLock);
		_messageQ.push(message);
		return message;
	}
	catch (...)	{
		if (message != nullptr)
			delete message;
		else
			LOG(Log::log("Failed to create message:" + addMssg);)
		return nullptr;
	}


}

const Message* Message::makeRemMsg(const SApair& saPair)
{
	auto remMssg = "[-] " + saPair.toString();
	Message* message = nullptr;
	try {
		message = new Message(remMssg);
		std::lock_guard<std::mutex> lock(_QinsLock);
		_messageQ.push(message);
		return message;
	}
	catch (...) {
		if (message != nullptr)
			delete message;
		else
			LOG(Log::log("Failed to create message:" + remMssg);)
		return nullptr;
	}
}

const Message* Message::makeBrdMsg(const SApair& saPair, const std::string_view& messageStr)
{
	auto brdMssg = "[M] " + saPair.toString() + " ";
	brdMssg += messageStr;

	Message* message = nullptr;
	try {
		message = new Message(brdMssg);
		std::lock_guard<std::mutex> lock(_QinsLock);
		_messageQ.push(message);
		return message;
	}
	catch (...) {
		if (message != nullptr)
			delete message;
		else
			LOG(Log::log("Failed to create message:" + brdMssg);)
		return nullptr;
	}
}

void Message::cleanMessageQ()
{
	std::lock_guard<std::mutex> lock(_QremLock);
	while (_messageQ.size() > MAX_MSG_CACHE_SIZE)
	{
		auto message = _messageQ.front();
		if (message->haveExpired())
		{
			_messageQ.pop();
			delete message;
		}
		else
			break;
	}
}
