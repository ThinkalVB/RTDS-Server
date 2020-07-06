#include "bg_controller.h"
#include <algorithm>
#include "log.h"

BGroupUnrestricted::BGroupUnrestricted(const std::string& bgID)
{
	mBgID = bgID;
}

void BGroupUnrestricted::addPeer(StreamPeer* peer)
{
	std::lock_guard<std::shared_mutex> writeLock(mTCPpeerListLock);
	mTCPpeerList.push_back(peer);
}

void BGroupUnrestricted::removePeer(StreamPeer* peer)
{
	std::lock_guard<std::shared_mutex> writeLock(mTCPpeerListLock);
	auto itr = std::find(mTCPpeerList.begin(), mTCPpeerList.end(), peer);
	std::iter_swap(itr, mTCPpeerList.end() - 1);
	mTCPpeerList.pop_back();
}

bool BGroupUnrestricted::isEmpty() const
{
	if (mTCPpeerList.size() == 0)
		return true;
	else
		return false;
}

void BGroupUnrestricted::broadcast(const Message* message)
{
	std::shared_lock<std::shared_mutex> readLock(mTCPpeerListLock);
	for (auto peer : mTCPpeerList)
			peer->sendMessage(message);
}


void BGroup::broadcast(StreamPeer* mPeer, const Message* message)
{
	std::shared_lock<std::shared_mutex> readLock(mTCPpeerListLock);
	for (auto peer : mTCPpeerList)
	{
		if (peer != mPeer)
			peer->sendMessage(message);
	}
}


std::map<std::string, BGroupUnrestricted*> BGcontroller::mBGmap;
std::shared_mutex BGcontroller::mBgLock;

BGroup* BGcontroller::addToBG(StreamPeer* peer, const BGID& bgID)
{
	std::lock_guard<std::shared_mutex> writeLock(mBgLock);
	auto bGroupItr = mBGmap.find(bgID);
	if (bGroupItr == mBGmap.end())
	{
		BGroupUnrestricted* bGroup = nullptr;
		try {
			auto bGroup = new BGroupUnrestricted(bgID);
			DEBUG_LOG(Log::log("Created BG: ", bgID);)
			bGroup->addPeer(peer);
			DEBUG_LOG(Log::log("Added peer to BG: ", bgID);)
			mBGmap.insert(std::pair(bgID, bGroup));
			DEBUG_LOG(Log::log("Added BG to map: ", bgID);)
			return (BGroup*)bGroup;
		}
		catch (const std::runtime_error& ec)
		{
			DEBUG_LOG(Log::log("Failed to create BG: ", bgID);)
			if (bGroup != nullptr)
				delete bGroup;
			return nullptr;
		}
	}
	else
	{
		try {
			auto bGroup = bGroupItr->second;
			bGroup->addPeer(peer);
			DEBUG_LOG(Log::log("Added peer to BG: ", bgID);)
			return (BGroup*)bGroup;
		}
		catch (const std::runtime_error& ec)
		{
			DEBUG_LOG(Log::log("Failed to add peer to BG: ", bgID);)
			return nullptr;
		}
	}
}

void BGcontroller::removeFromBG(StreamPeer* peer, const BGID& bgID)
{
	std::lock_guard<std::shared_mutex> writeLock(mBgLock);
	auto bGroupItr = mBGmap.find(bgID);
	if (bGroupItr != mBGmap.end())
	{
		auto bGroup = bGroupItr->second;
		bGroup->removePeer(peer);
		if (bGroup->isEmpty())
		{
			mBGmap.erase(bGroupItr);
			DEBUG_LOG(Log::log(bgID, " Deleted BG: ", bgID);)
		}
	}
}

void BGcontroller::broadcast(const Message* message, const BGID bgID)
{
	std::shared_lock<std::shared_mutex> readLock(mBgLock);
	auto bGroupItr = mBGmap.find(bgID);
	if (bGroupItr != mBGmap.end())
	{
		auto bGroup = bGroupItr->second;
		bGroup->broadcast(message);
	}
}
