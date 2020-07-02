#include "bg_controller.h"
#include <algorithm>
#include "log.h"

BGroupUnrestricted::BGroupUnrestricted(const std::string& bgID)
{
	mBgID = bgID;
}

void BGroupUnrestricted::addPeer(TCPpeer* peer)
{
	std::lock_guard<std::shared_mutex> writeLock(mTCPpeerListLock);
	mTCPpeerList.push_back(peer);
}

void BGroupUnrestricted::removePeer(TCPpeer* peer)
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

void BGroupUnrestricted::broadcast(const Message* message, const std::string_view& bgTag)
{
	std::shared_lock<std::shared_mutex> readLock(mTCPpeerListLock);
	for (auto peer : mTCPpeerList)
		peer->sendMessage(message, bgTag);
}

void BGroupUnrestricted::broadcast(const Message* message)
{
	std::shared_lock<std::shared_mutex> readLock(mTCPpeerListLock);
	for (auto peer : mTCPpeerList)
			peer->sendMessage(message);
}


void BGroup::broadcast(TCPpeer* mPeer, const Message* message, const std::string_view& bgTag)
{
	std::shared_lock<std::shared_mutex> readLock(mTCPpeerListLock);
	for (auto peer : mTCPpeerList)
	{
		if (peer != mPeer)
			peer->sendMessage(message, bgTag);
	}
}

void BGroup::broadcast(TCPpeer* mPeer, const Message* message)
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

BGroup* BGcontroller::addToBG(TCPpeer* peer, const BGID& bgID)
{
	std::lock_guard<std::shared_mutex> writeLock(mBgLock);
	auto bGroupItr = mBGmap.find(bgID);
	if (bGroupItr == mBGmap.end())
	{
		auto bGroup = new (std::nothrow) BGroupUnrestricted(bgID);
		if (bGroup == nullptr)
		{
			LOG(Log::log("Failed to create BG: ", bgID);)
			return nullptr;
		}
		else
		{
			try {
				bGroup->addPeer(peer);
				mBGmap.insert(std::pair(bgID, bGroup));
				DEBUG_LOG(Log::log("Added BG: ", bgID);)
				return (BGroup*)bGroup;
			}
			catch (...) {
				LOG(Log::log("Failed to add peer to BG!");)
				delete bGroup;
				return nullptr;
			}
		}
	}
	else
	{
		try {
			auto bGroup = bGroupItr->second;
			bGroup->addPeer(peer);
			return (BGroup*)bGroup;
		}
		catch (...) {
			LOG(Log::log("Failed to add peer to BG! ");)
			return nullptr;
		}
	}
}

void BGcontroller::removeFromBG(TCPpeer* peer, const BGID& bgID)
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
	else
	{	
		LOG(Log::log("Peer must be in map, but not found");)
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

void BGcontroller::broadcast(const Message* message, const BGID bgID, const std::string_view& bgTag)
{
	std::shared_lock<std::shared_mutex> readLock(mBgLock);
	auto bGroupItr = mBGmap.find(bgID);
	if (bGroupItr != mBGmap.end())
	{
		auto bGroup = bGroupItr->second;
		bGroup->broadcast(message, bgTag);
	}
}
