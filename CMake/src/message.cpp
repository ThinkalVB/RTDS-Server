#include "message.h"
#include "rtds_settings.h"
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

const Message* Message::makeAddMsg(const SApair& saPair, const std::string& bgTag)
{
	auto addMssg = "[C] " + saPair.toString() + " " + bgTag;
	Message* message = nullptr;
	try {
		message = new Message(addMssg);
		std::lock_guard<std::mutex> lock(mQinsLock);
		mMessageQ.push(message);
		mCleanMessageQ();
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
		std::lock_guard<std::mutex> lock(mQinsLock);
		mMessageQ.push(message);
		mCleanMessageQ();
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
		std::lock_guard<std::mutex> lock(mQinsLock);
		mMessageQ.push(message);
		mCleanMessageQ();
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