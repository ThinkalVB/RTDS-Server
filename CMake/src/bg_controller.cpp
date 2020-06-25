#include "bg_controller.h"
#include "rtds_settings.h"
#include "log.h"

BGroupUnrestricted::BGroupUnrestricted(const std::string& bgID)
{
	mBgID = bgID;
}

void BGroupUnrestricted::addPeer(Peer* peer)
{
	mPeerListLock.lock();
	mPeerList.push_back(peer);
	mPeerListLock.unlock();
}

void BGroupUnrestricted::removePeer(Peer* peer)
{
	mPeerListLock.lock();
	auto itr = std::find(mPeerList.begin(), mPeerList.end(), peer);
	std::iter_swap(itr, mPeerList.end() - 1);
	mPeerList.pop_back();
	mPeerListLock.unlock();
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
	mPeerListLock.lock_shared();
	for (auto peer : mPeerList)
	{
		if (peer != mPeer)
			peer->sendMessage(message, bgTag);
	}
	mPeerListLock.unlock_shared();
}

void BGroup::broadcast(Peer* mPeer, const Message* message)
{
	mPeerListLock.lock_shared();
	for (auto peer : mPeerList)
	{
		if (peer != mPeer)
			peer->sendMessage(message);
	}
	mPeerListLock.unlock_shared();
}


std::map<std::string, BGroupUnrestricted*> BGcontroller::mBGmap;
std::mutex BGcontroller::mBgLock;

BGroup* BGcontroller::addToBG(Peer* peer, const BGID& bgID)
{
	std::lock_guard<std::mutex> lock(mBgLock);
	auto bGroupItr = mBGmap.find(bgID);
	if (bGroupItr == mBGmap.end())
	{
		auto bGroup = new (std::nothrow) BGroupUnrestricted(bgID);
		if (bGroup == nullptr)
		{
			LOG(Log::log("Failed to create BG: ", bgID);)
			REGISTER_MEMMORY_ERR
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
				REGISTER_MEMMORY_ERR
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
