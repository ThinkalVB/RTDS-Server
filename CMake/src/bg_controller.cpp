#include "bg_controller.h"
#include "rtds_settings.h"
#include "log.h"

BGroupUnrestricted::BGroupUnrestricted(const std::string& bgID)
{
	mBgID = bgID;
}

void BGroupUnrestricted::addPeer(Peer* peer)
{
	std::lock_guard<std::mutex> lock(mPeerListLock);
	mPeerList.push_back(peer);
}

void BGroupUnrestricted::removePeer(Peer* peer)
{
	std::lock_guard<std::mutex> lock(mPeerListLock);
	auto itr = std::find(mPeerList.begin(), mPeerList.end(), peer);
	std::iter_swap(itr, mPeerList.end() - 1);
	mPeerList.pop_back();
}

bool BGroupUnrestricted::isEmpty() const
{
	if (mPeerList.size() == 0)
		return true;
	else
		return false;
}


void BGroup::broadcast(Peer* mPeer, const Message* message, const std::string_view& bgTag)
{
	std::lock_guard<std::mutex> lock(mPeerListLock);
	for (auto peer : mPeerList)
	{
		if (peer != mPeer)
			peer->sendMessage(message, bgTag);
	}
}

void BGroup::broadcast(Peer* mPeer, const Message* message)
{
	std::lock_guard<std::mutex> lock(mPeerListLock);
	for (auto peer : mPeerList)
	{
		if (peer != mPeer)
			peer->sendMessage(message);
	}
}


std::map<std::string, BGroupUnrestricted*> BGcontroller::mBGmap;
std::mutex BGcontroller::mBgLock;

BGroup* BGcontroller::addToBG(Peer* peer, const BGID& bgID)
{
	std::lock_guard<std::mutex> lock(mBgLock);
	auto bGroupItr = mBGmap.find(bgID);
	if (bGroupItr == mBGmap.end())
	{
		BGroupUnrestricted* bGroup = nullptr;
		try {
			bGroup = new BGroupUnrestricted(bgID);
			bGroup->addPeer(peer);
			mBGmap.insert(std::pair(bgID, bGroup));
			DEBUG_LOG(Log::log("Added BG: ", bgID);)
			return (BGroup*)bGroup;
		}
		catch (...) {
			if (bGroup != nullptr)
			{
				LOG(Log::log("Failed to add BG: ", bgID);)
				delete bGroup;
			}
			else
			{	LOG(Log::log("Failed to add peer to BG");)	}
			REGISTER_MEMMORY_ERR
			return nullptr;
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
			LOG(Log::log("Failed to add peer to BG");)
			REGISTER_MEMMORY_ERR
			return nullptr;
		}
	}
}

void BGcontroller::removeFromBG(Peer* peer, const BGID& bgID)
{
	std::lock_guard<std::mutex> lock(mBgLock);
	auto bGroupItr = mBGmap.find(bgID);
	if (bGroupItr != mBGmap.end())
	{
		auto bGroup = bGroupItr->second;
		bGroup->removePeer(peer);
		if (bGroup->isEmpty())
		{
			mBGmap.erase(bGroupItr);
			DEBUG_LOG(Log::log(bgID, "Deleted BG: ", bgID);)
		}
	}
	else
	{	
		LOG(Log::log("Peer must be in map, but not found");)
		REGISTER_CODE_ERROR
	}
}
