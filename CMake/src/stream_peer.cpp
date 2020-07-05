#include "stream_peer.h"
#include "cmd_processor.h"
#include "bg_controller.h"
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


const PeerType StreamPeer::peerType() const
{
	return mPeerType;
}

int StreamPeer::getPeerCount() const
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

void StreamPeer::listenTo(const std::string_view& bgID, const std::string_view& bgTag)
{
	std::string response = "[R]\t";
	if (mIsInBG)
		response += CmdProcessor::RESP[(short)Response::IS_IN_BG];
	else
	{
		mBgID = bgID;
		mBgTag = bgTag;

		mBgPtr = BGcontroller::addToBG(this, mBgID);
		if (mBgPtr != nullptr)
		{
			mIsInBG = true;
			mPeerMode = PeerMode::LISTEN;

			auto message = Message::makeAddMsg(mSApair, mBgTag, mPeerType);
			if (message != nullptr)
				mBgPtr->broadcast(this, message);

			response += CmdProcessor::RESP[(short)Response::SUCCESS];
			DEBUG_LOG(Log::log(mSApair, " Listening to Tag: ", mBgTag, " BG: ", mBgID);)
		}
		else
		{
			response += CmdProcessor::RESP[(short)Response::WAIT_RETRY];
			LOG(Log::log(mSApair, " Failed to create joining message!");)
		}
	}
	response += "\n";
	mDataBuffer = response;
}

void StreamPeer::leaveBG()
{
	std::string response = "[R]\t";
	if (mIsInBG)
	{
		DEBUG_LOG(Log::log(mSApair, " Peer leavig BG ", mBgID);)
			BGcontroller::removeFromBG(this, mBgID);

		if (mPeerMode == PeerMode::LISTEN)
		{
			auto message = Message::makeRemMsg(mSApair, mBgTag, mPeerType);
			if (message != nullptr)
				mBgPtr->broadcast(this, message);
			else
			{
				LOG(Log::log(mSApair, " Failed to create leaving message!");)
			}
		}

		mIsInBG = false;
		mBgPtr = nullptr;
		response += CmdProcessor::RESP[(short)Response::SUCCESS];
	}
	else
		response += CmdProcessor::RESP[(short)Response::NOT_IN_BG];
	response += "\n";
	mDataBuffer = response;
}

void StreamPeer::broadcastTo(const std::string_view& messageStr, const std::string_view& bgTag)
{
	std::string response = "[R]\t";
	if (mIsInBG)
	{
		const Message* message;
		if (bgTag == OWN_TAG)
			message = Message::makeBrdMsg(messageStr, mBgTag, mPeerType);
		else
			message = Message::makeBrdMsg(messageStr, bgTag, mPeerType);

		if (message != nullptr)
		{
			mBgPtr->broadcast(this, message);
			response += CmdProcessor::RESP[(short)Response::SUCCESS];
			DEBUG_LOG(Log::log(mSApair, " Peer broadcasting: ", messageStr);)
		}
		else
		{
			response += CmdProcessor::RESP[(short)Response::WAIT_RETRY];
			LOG(Log::log(mSApair, " Failed to create message!");)
		}
	}
	else
		response += CmdProcessor::RESP[(short)Response::NOT_IN_BG];
	response += "\n";
	mDataBuffer = response;
}

void StreamPeer::messageTo(const std::string_view& messageStr, const std::string_view& bgTag)
{
	std::string response = "[R]\t";
	if (mIsInBG)
	{
		const Message* message;
		if (bgTag == OWN_TAG)
			message = Message::makeMsg(mSApair, messageStr, mBgTag, mPeerType);
		else
			message = Message::makeMsg(mSApair, messageStr, bgTag, mPeerType);

		if (message != nullptr)
		{
			mBgPtr->broadcast(this, message);
			response += CmdProcessor::RESP[(short)Response::SUCCESS];
			DEBUG_LOG(Log::log(mSApair, " Peer broadcasting: ", messageStr);)
		}
		else
		{
			response += CmdProcessor::RESP[(short)Response::WAIT_RETRY];
			LOG(Log::log(mSApair, " Failed to create message!");)
		}
	}
	else
		response += CmdProcessor::RESP[(short)Response::NOT_IN_BG];
	response += "\n";
	mDataBuffer = response;
}
