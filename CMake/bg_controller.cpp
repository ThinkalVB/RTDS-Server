#include "bg_controller.h"
#include "log.h"

BGroup_Unrestricted::BGroup_Unrestricted(const std::string& bgID)
{
	_bgID = bgID;
}

void BGroup_Unrestricted::addPeer(Peer* peer)
{
	std::lock_guard<std::mutex> lock(_peerListLock);
	_peerList.push_back(peer);
}

void BGroup_Unrestricted::removePeer(Peer* peer)
{
	std::lock_guard<std::mutex> lock(_peerListLock);
	auto itr = std::find(_peerList.begin(), _peerList.end(), peer);
	std::iter_swap(itr, _peerList.end() - 1);
	_peerList.pop_back();
}

bool BGroup_Unrestricted::isEmpty() const
{
	if (_peerList.size() == 0)
		return true;
	else
		return false;
}


void BGroup::broadcast(Peer* mPeer, const Message* message, const std::string_view& bgTag)
{
	std::lock_guard<std::mutex> lock(_peerListLock);
	for (auto peer : _peerList)
	{
		if (peer != mPeer)
			peer->sendMessage(message, bgTag);
	}
}

void BGroup::broadcast(Peer* mPeer, const Message* message)
{
	std::lock_guard<std::mutex> lock(_peerListLock);
	for (auto peer : _peerList)
	{
		if (peer != mPeer)
			peer->sendMessage(message);
	}
}


std::map<std::string, BGroup_Unrestricted*> BGcontroller::_BGmap;
std::mutex BGcontroller::_bgLock;

BGroup* BGcontroller::addToBG(Peer* peer, const BGID& bgID)
{
	std::lock_guard<std::mutex> lock(_bgLock);
	auto bGroupItr = _BGmap.find(bgID);
	if (bGroupItr == _BGmap.end())
	{
		BGroup_Unrestricted* bGroup = nullptr;
		try {
			bGroup = new BGroup_Unrestricted(bgID);
			bGroup->addPeer(peer);
			_BGmap.insert(std::pair(bgID, bGroup));
			DEBUG_LOG(Log::log(bgID, " BG added");)
			return (BGroup*)bGroup;
		}
		catch (...) {
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
			return (BGroup*)bGroup;
		}
		catch (...) {
			return nullptr;
		}
	}
}

void BGcontroller::removeFromBG(Peer* peer, const BGID& bgID)
{
	std::lock_guard<std::mutex> lock(_bgLock);
	auto bGroupItr = _BGmap.find(bgID);
	if (bGroupItr != _BGmap.end())
	{
		auto bGroup = bGroupItr->second;
		bGroup->removePeer(peer);
		if (bGroup->isEmpty())
		{
			_BGmap.erase(bGroupItr);
			DEBUG_LOG(Log::log(bgID, " BG deleted");)
		}
	}
	else
		LOG(Log::log("Peer must be in map, but not found");)
}
