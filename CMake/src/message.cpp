#include "message.h"
#include "rtds_ccm.h"
#include "common.h"
#include "log.h"

std::queue<Message*> Message::mMessageQ;
std::mutex Message::mQinsLock;
std::mutex Message::mQremLock;

Message::Message(const std::string& messageStr)
{
	DEBUG_LOG(Log::log("New message created: ", messageStr);)
	mCreatedTime = std::chrono::system_clock::now();
	messageBuf = messageStr;
}

bool Message::haveExpired() const
{
	TimePoint timeNow = std::chrono::system_clock::now();
	auto diffTime = std::chrono::duration_cast<std::chrono::minutes>(timeNow - mCreatedTime);
	auto timePassed = diffTime.count();
	if (timePassed > MIN_MSG_KEEP_TIME)
		return true;
	else
		return false;
}

const Message* Message::makeAddMsg(const SApair& saPair)
{
	auto addMssg = "[C]\t" + saPair.toString() + "\n";
	auto message = new (std::nothrow) Message(addMssg);
	if (message == nullptr)
	{
		LOG(Log::log("Failed to create message! ", addMssg);)
		REGISTER_MEMMORY_ERR
		return nullptr;
	}
	else
	{
		if (!insertMssg2Q(message))
			return nullptr;
		else
			return message;
	}
}

const Message* Message::makeRemMsg(const SApair& saPair)
{
	auto remMssg = "[D]\t" + saPair.toString() + "\n";
	auto message = new (std::nothrow) Message(remMssg);
	if (message == nullptr)
	{
		LOG(Log::log("Failed to create message! ", remMssg);)
		REGISTER_MEMMORY_ERR
		return nullptr;
	}
	else
	{
		if (!insertMssg2Q(message))
			return nullptr;
		else
			return message;
	}
}

const Message* Message::makeBrdMsg(const SApair& saPair, const std::string_view& messageStr)
{
	auto brdMssg = "[M]\t" + saPair.toString() + "\t";
	brdMssg += messageStr;
	brdMssg += "\n";

	auto message = new (std::nothrow) Message(brdMssg);
	if (message == nullptr)
	{
		LOG(Log::log("Failed to create message! ", brdMssg);)
		REGISTER_MEMMORY_ERR
		return nullptr;
	}
	else
	{
		if (!insertMssg2Q(message))
			return nullptr;
		else
			return message;
	}
}

const Message* Message::makeBrdMsg(const std::string saPairStr, const std::string_view& messageStr)
{
	auto brdMssg = "[W]\t" + saPairStr + "\t";
	brdMssg += messageStr;
	brdMssg += "\n";

	auto message = new (std::nothrow) Message(brdMssg);
	if (message == nullptr)
	{
		LOG(Log::log("Failed to create message! ", brdMssg);)
		REGISTER_MEMMORY_ERR
		return nullptr;
	}
	else
	{
		if (!insertMssg2Q(message))
			return nullptr;
		else
			return message;
	}
}

bool Message::insertMssg2Q(Message* message)
{
	try
	{
		mCleanMessageQ();
		std::lock_guard<std::mutex> lock(mQinsLock);
		mMessageQ.push(message);
		return true;
	}catch (...) {
		LOG(Log::log("Failed to add message to Queue!");)
		REGISTER_MEMMORY_ERR
		delete message;
		return false;
	}
}

void Message::mCleanMessageQ()
{
	if (mQremLock.try_lock())
	{
		while (mMessageQ.size() > MAX_MSG_CACHE_SIZE)
		{
			auto message = mMessageQ.front();
			if (message->haveExpired())
			{
				DEBUG_LOG(Log::log("Message deleted ", message->messageBuf);)
				mMessageQ.pop();
				delete message;
			}
			else
				break;
		}
		mQremLock.unlock();
	}
}