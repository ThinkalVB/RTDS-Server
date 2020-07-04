#include "stream_peer.h"
#include "cmd_processor.h"
#include "log.h"

std::atomic_int StreamPeer::mGlobalPeerCount;

StreamPeer::StreamPeer()
{
	mPeerIsActive = true;
	mBgPtr = nullptr;
	mIsInBG = false;
	mGlobalPeerCount++;
}

StreamPeer::~StreamPeer()
{
	mGlobalPeerCount--;
}

int StreamPeer::getPeerCount()
{
	return mGlobalPeerCount;
}

void StreamPeer::disconnect()
{
	DEBUG_LOG(Log::log(mSApair, " Peer Disconnecting");)
	mPeerIsActive = false;
}

void StreamPeer::changeTag(const std::string_view& bgTag)
{
	std::string response = "[R]\t";
	if (!mIsInBG)
		response += CmdProcessor::RESP[(short)Response::NOT_IN_BG];
	else
	{
		std::lock_guard<std::shared_mutex> writeLock(mPeerResourceMtx);
		mBgTag = bgTag;

		response += CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::log(mSApair, " Changed Tag to: ", mBgTag);)
	}
	response += "\n";
	mDataBuffer = response;
}

void StreamPeer::printPingInfo()
{
	std::string response = "[R]\t";
	response += mSApair;
	response += "\n";
	DEBUG_LOG(Log::log(mSApair, " Peer pinging");)
	mDataBuffer = response;
}

void StreamPeer::respondWith(const Response resp)
{
	std::string response = "[R]\t";
	response += CmdProcessor::RESP[(short)resp];
	response += "\n";
	DEBUG_LOG(Log::log(mSApair, " Peer responding: ", CmdProcessor::RESP[(short)resp]);)
	mDataBuffer = response;
}
