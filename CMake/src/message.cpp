#include "message.h"
#include "common.h"
#include "log.h"

std::queue<Message*> Message::mMessageQ;
std::mutex Message::mQinsLock;
std::mutex Message::mQremLock;

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