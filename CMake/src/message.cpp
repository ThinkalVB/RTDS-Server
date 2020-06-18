#include "message.h"
#include "rtds_settings.h"
#include "common.h"
#include "log.h"

std::queue<Message*> Message::m_messageQ;
std::mutex Message::m_QinsLock;
std::mutex Message::m_QremLock;

Message::Message(const std::string& messageStr)
{
	DEBUG_LOG(Log::log("New message created: ", messageStr);)
	m_createdTime = std::chrono::system_clock::now();
	messageBuf = messageStr;
}

bool Message::haveExpired() const
{
	TimePoint timeNow = std::chrono::system_clock::now();
	auto diffTime = std::chrono::duration_cast<std::chrono::minutes>(timeNow - m_createdTime);
	auto timePassed = diffTime.count();
	if (timePassed > MIN_MSG_KEEP_TIME)
		return true;
	else
		return false;
}

const Message* Message::makeAddMsg(const SApair& saPair, const std::string& bgTag)
{
	auto addMssg = "[C] " + saPair.toString() + " " + bgTag;
	Message* message = nullptr;
	try {
		message = new Message(addMssg);
		std::lock_guard<std::mutex> lock(m_QinsLock);
		m_messageQ.push(message);
		m_cleanMessageQ();
		return message;
	}
	catch (...)	{
		if (message != nullptr)
		{
			LOG(Log::log("Failed to add message to Q: ");)
			delete message;
		}
		else
		{	LOG(Log::log("Failed to create message: ", addMssg);)	}
		REGISTER_MEMMORY_ERR
		return nullptr;
	}
}

const Message* Message::makeRemMsg(const SApair& saPair, const std::string& bgTag)
{
	auto remMssg = "[D] " + saPair.toString() + " " + bgTag;
	Message* message = nullptr;
	try {
		message = new Message(remMssg);
		std::lock_guard<std::mutex> lock(m_QinsLock);
		m_messageQ.push(message);
		m_cleanMessageQ();
		return message;
	}
	catch (...) {
		if (message != nullptr)
		{
			LOG(Log::log("Failed to add message to Q: ");)
			delete message;
		}
		else
		{	LOG(Log::log("Failed to create message: ", remMssg);)	}
		REGISTER_MEMMORY_ERR
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
		std::lock_guard<std::mutex> lock(m_QinsLock);
		m_messageQ.push(message);
		m_cleanMessageQ();
		return message;
	}
	catch (...) {
		if (message != nullptr)
		{
			LOG(Log::log("Failed to add message to Q: ");)
			delete message;
		}
		else
		{	LOG(Log::log("Failed to create message: ", brdMssg);)	}
		REGISTER_MEMMORY_ERR
		return nullptr;
	}
}

void Message::m_cleanMessageQ()
{
	if (m_QremLock.try_lock())
	{
		while (m_messageQ.size() > MAX_MSG_CACHE_SIZE)
		{
			auto message = m_messageQ.front();
			if (message->haveExpired())
			{
				DEBUG_LOG(Log::log("Message deleted ", message->messageBuf);)
				m_messageQ.pop();
				delete message;
			}
			else
				break;
		}
		m_QremLock.unlock();
	}
}