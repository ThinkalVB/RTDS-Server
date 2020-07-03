#include "tcp_peer.h"
#include <functional>
#include "cmd_processor.h"
#include "bg_controller.h"
#include "log.h"

std::atomic_int TCPpeer::mPeerCount = 0;

TCPpeer::TCPpeer(asio::ip::tcp::socket* socketPtr) : mSApair(socketPtr)
{
	mPeerSocket = socketPtr;
	mPeerIsActive = true;
	mBgPtr = nullptr;
	mIsInBG = false;

	DEBUG_LOG(Log::log(mSApair.toString()," TCP Peer Connected");)
	mPeerCount++;
	mPeerReceiveData();
}

int TCPpeer::peerCount()
{
	return mPeerCount;
}

TCPpeer::~TCPpeer()
{
	leaveBG();
	mPeerCount--;
	mPeerSocket->shutdown(asio::ip::tcp::socket::shutdown_both);

	asio::error_code ec;
	mPeerSocket->close(ec);
	if (ec)
	{	LOG(Log::log(mSApair.toString(), " socket cannot close - ", ec.message());)		}
	delete mPeerSocket;
	DEBUG_LOG(Log::log(mSApair.toString(), " TCP Peer Disconnected");)
}

std::string_view TCPpeer::getCommandString()
{
	return mDataBuffer.getStringView();
}


void TCPpeer::sendMessage(const Message* message, const std::string_view& bgTag)
{
	std::shared_lock<std::shared_mutex> readLock(mPeerResourceMtx);
	if (mPeerIsActive && mBgTag == bgTag)
	{
		mPeerSocket->async_send(asio::buffer(message->messageBuf.data(), message->messageBuf.size()),
			std::bind(&TCPpeer::mSendMssgFuncFeedbk, this, std::placeholders::_1));
	}
}

void TCPpeer::sendMessage(const Message* message)
{
	if (mPeerIsActive)
	{
		mPeerSocket->async_send(asio::buffer(message->messageBuf.data(), message->messageBuf.size()),
			std::bind(&TCPpeer::mSendMssgFuncFeedbk, this, std::placeholders::_1));
	}
}


void TCPpeer::mSendFuncFeedbk(const asio::error_code& ec)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(mSApair.toString(), " Peer socket _sendData() failed", ec.message());)
		delete this;
	}
	else
		mPeerReceiveData();
}

void TCPpeer::mSendMssgFuncFeedbk(const asio::error_code& ec)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(mSApair.toString(), " Peer socket _sendMessage() failed", ec.message());)
		mPeerIsActive = false;
	}
}


void TCPpeer::mSendPeerBufferData()
{
	mPeerSocket->async_send(mDataBuffer.getSendBuffer(), std::bind(&TCPpeer::mSendFuncFeedbk,
		this, std::placeholders::_1));
}

void TCPpeer::mPeerReceiveData()
{
	if (mPeerIsActive)
	{
		mPeerSocket->async_receive(mDataBuffer.getReadBuffer(), 0, std::bind(&TCPpeer::mProcessData,
			this, std::placeholders::_1, std::placeholders::_2));
	}
	else
		delete this;
}

void TCPpeer::mProcessData(const asio::error_code& ec, std::size_t dataSize)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(mSApair.toString(), " Peer socket _processData() failed ", ec.message());)
		delete this;
	}
	else
	{
		if (mDataBuffer.cookString(dataSize))
			CmdProcessor::processCommand(*this);
		else
			respondWith(Response::BAD_COMMAND);

		if (mPeerIsActive)
			mSendPeerBufferData();
		else
			delete this;
	}
}


void TCPpeer::disconnect()
{
	DEBUG_LOG(Log::log(mSApair.toString(), " Peer Disconnecting");)
	mPeerIsActive = false;
}

void TCPpeer::listenTo(const std::string_view& bgID, const std::string_view& bgTag)
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

			auto message = Message::makeAddMsg(mSApair, mBgTag, mBgTag, PeerType::TCP);
			if (message != nullptr)
				mBgPtr->broadcast(this, message);

			response += CmdProcessor::RESP[(short)Response::SUCCESS];
			DEBUG_LOG(Log::log(mSApair.toString(), " Listening to Tag: ", mBgTag, " BG: ", mBgID);)
		}
		else
		{
			response += CmdProcessor::RESP[(short)Response::WAIT_RETRY];
			LOG(Log::log(mSApair.toString(), " Failed to create joining message!");)
		}
	}
	response += "\n";
	mDataBuffer = response;
}

void TCPpeer::hearTo(const std::string_view& bgID, const std::string_view& bgTag)
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
			mPeerMode = PeerMode::HEAR;

			response += CmdProcessor::RESP[(short)Response::SUCCESS];
			DEBUG_LOG(Log::log(mSApair.toString(), " Listening to Tag: ", mBgTag, " BG: ", mBgID);)
		}
		else
			response += CmdProcessor::RESP[(short)Response::WAIT_RETRY];
	}
	response += "\n";
	mDataBuffer = response;
}

void TCPpeer::leaveBG()
{
	std::string response = "[R]\t";
	if (mIsInBG)
	{
		DEBUG_LOG(Log::log(mSApair.toString(), " Peer leavig BG ", mBgID);)
		BGcontroller::removeFromBG(this, mBgID);

		if (mPeerMode == PeerMode::LISTEN)
		{
			auto message = Message::makeRemMsg(mSApair, mBgTag, mBgTag, PeerType::TCP);
			if (message != nullptr)
				mBgPtr->broadcast(this, message);
			else
			{	LOG(Log::log(mSApair.toString(), " Failed to create leaving message!");)	}
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

void TCPpeer::changeTag(const std::string_view& bgTag)
{
	std::string response = "[R]\t";
	if (!mIsInBG)
		response += CmdProcessor::RESP[(short)Response::NOT_IN_BG];
	else
	{
		std::lock_guard<std::shared_mutex> writeLock(mPeerResourceMtx);
		mBgTag = bgTag;

		response += CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::log(mSApair.toString(), " Changed Tag to: ", mBgTag);)
	}
	response += "\n";
	mDataBuffer = response;
}

void TCPpeer::printPingInfo()
{
	std::string response = "[R]\t";
	DEBUG_LOG(Log::log(mSApair.toString(), " Peer pinging");)
	response += mSApair.toString();
	response += "\n";
	mDataBuffer = response;
}

void TCPpeer::respondWith(Response resp)
{
	std::string response = "[R]\t";
	DEBUG_LOG(Log::log(mSApair.toString(), " Peer responding: ", CmdProcessor::RESP[(short)resp]);)
	response += CmdProcessor::RESP[(short)resp];
	response += "\n";
	mDataBuffer = response;
}

void TCPpeer::broadcastTo(const std::string_view& messageStr, const std::string_view& bgTag)
{
	std::string response = "[R]\t";
	if (mIsInBG)
	{
		auto message = Message::makeBrdMsg(mSApair.toString(), messageStr, mBgTag, bgTag, PeerType::TCP);
		if (message != nullptr)
		{
			mBgPtr->broadcast(this, message, bgTag);
			response += CmdProcessor::RESP[(short)Response::SUCCESS];
			DEBUG_LOG(Log::log(mSApair.toString(), " Peer broadcasting: ", messageStr);)
		}
		else
		{
			response += CmdProcessor::RESP[(short)Response::WAIT_RETRY];
			LOG(Log::log(mSApair.toString(), " Failed to create message!");)
		}
	}
	else
		response += CmdProcessor::RESP[(short)Response::NOT_IN_BG];
	response += "\n";
	mDataBuffer = response;
}
