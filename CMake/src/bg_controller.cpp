#include "bg_controller.h"
#include "rtds_settings.h"
#include "log.h"

BGroupUnrestricted::BGroupUnrestricted(const std::string& bgID)
{
	m_bgID = bgID;
}

void BGroupUnrestricted::addPeer(Peer* peer)
{
	std::lock_guard<std::mutex> lock(m_peerListLock);
	m_peerList.push_back(peer);
}

void BGroupUnrestricted::removePeer(Peer* peer)
{
	std::lock_guard<std::mutex> lock(m_peerListLock);
	auto itr = std::find(m_peerList.begin(), m_peerList.end(), peer);
	std::iter_swap(itr, m_peerList.end() - 1);
	m_peerList.pop_back();
}

bool BGroupUnrestricted::isEmpty() const
{
	if (m_peerList.size() == 0)
		return true;
	else
		return false;
}


void BGroup::broadcast(Peer* mPeer, const Message* message, const std::string_view& bgTag)
{
	std::lock_guard<std::mutex> lock(m_peerListLock);
	for (auto peer : m_peerList)
	{
		if (peer != mPeer)
			peer->sendMessage(message, bgTag);
	}
}

void BGroup::broadcast(Peer* mPeer, const Message* message)
{
	std::lock_guard<std::mutex> lock(m_peerListLock);
	for (auto peer : m_peerList)
	{
		if (peer != mPeer)
			peer->sendMessage(message);
	}
}


std::map<std::string, BGroupUnrestricted*> BGcontroller::m_BGmap;
std::mutex BGcontroller::m_bgLock;

BGroup* BGcontroller::addToBG(Peer* peer, const BGID& bgID)
{
	std::lock_guard<std::mutex> lock(m_bgLock);
	auto bGroupItr = m_BGmap.find(bgID);
	if (bGroupItr == m_BGmap.end())
	{
		BGroupUnrestricted* bGroup = nullptr;
		try {
			bGroup = new BGroupUnrestricted(bgID);
			bGroup->addPeer(peer);
			m_BGmap.insert(std::pair(bgID, bGroup));
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
	std::lock_guard<std::mutex> lock(m_bgLock);
	auto bGroupItr = m_BGmap.find(bgID);
	if (bGroupItr != m_BGmap.end())
	{
		auto bGroup = bGroupItr->second;
		bGroup->removePeer(peer);
		if (bGroup->isEmpty())
		{
			m_BGmap.erase(bGroupItr);
			DEBUG_LOG(Log::log(bgID, "Deleted BG: ", bgID);)
		}
	}
	else
	{	
		LOG(Log::log("Peer must be in map, but not found");)
		REGISTER_CODE_ERROR
	}
}
