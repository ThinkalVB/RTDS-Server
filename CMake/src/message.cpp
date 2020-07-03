#include "message.h"
#include "common.h"
#include "log.h"

std::queue<Message*> Message::mMessageQ;
std::mutex Message::mQinsLock;
std::mutex Message::mQremLock;

/*
Message::Message(const std::string& messageStr)
{
	DEBUG_LOG(Log::log("New message created: ", messageStr);)
	mCreatedTime = std::chrono::system_clock::now();
	messageBuf = messageStr;
	peersTag = ALL_TAG;
}

Message::Message(const std::string& messageStr, const BGT& peerTag,const PeerType peerType)
{
	DEBUG_LOG(Log::log("New message created: ", messageStr);)
	mCreatedTime = std::chrono::system_clock::now();
	messageBuf = messageStr;
	peersTag = peersTag;
}
*/

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

const Message* Message::makeAddMsg(const SApair& saPair, const BGT& peerTag)
{
	/*
	auto addMssg = "[C]\t" + saPair.toString() + "\n";
	auto message = new (std::nothrow) Message(addMssg, peerTag);
	if (message == nullptr)
	{
		LOG(Log::log("Failed to create message! ", addMssg);)
		return nullptr;
	}
	else
	{
		if (!insertMssg2Q(message))
			return nullptr;
		else
			return message;
	}
	*/
	return nullptr;
}

const Message* Message::makeRemMsg(const SApair& saPair, const BGT& peerTag)
{
	/*
	auto remMssg = "[D]\t" + saPair.toString() + "\n";
	auto message = new (std::nothrow) Message(remMssg, peerTag);
	if (message == nullptr)
	{
		LOG(Log::log("Failed to create message! ", remMssg);)
		return nullptr;
	}
	else
	{
		if (!insertMssg2Q(message))
			return nullptr;
		else
			return message;
	}
	*/
	return nullptr;
}

const Message* Message::makeBrdMsg(const std::string saPairStr, const std::string_view& messageStr, const BGT& bgTag)
{
	/*
	auto brdMssg = "[W]\t" + saPairStr + "\t";
	brdMssg += messageStr;
	brdMssg += "\n";

	auto message = new (std::nothrow) Message(brdMssg, bgTag);
	if (message == nullptr)
	{
		LOG(Log::log("Failed to create message! ", brdMssg);)
		return nullptr;
	}
	else
	{
		if (!insertMssg2Q(message))
			return nullptr;
		else
			return message;
	}
	*/
	return nullptr;
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