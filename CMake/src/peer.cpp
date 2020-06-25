#include "peer.h"
#include <functional>
#include "cmd_processor.h"
#include "bg_controller.h"
#include "log.h"

std::atomic_int Peer::mPeerCount = 0;

Peer::Peer(asio::ip::tcp::socket* socketPtr) : mSApair(socketPtr)
{
	mPeerSocket = socketPtr;
	mPeerIsActive = true;
	mBgPtr = nullptr;
	mIsInBG = false;

	DEBUG_LOG(Log::log(mSApair.toString()," Peer Connected");)
	mPeerCount++;
	mPeerReceiveData();
}

int Peer::peerCount()
{
	return mPeerCount;
}

Peer::~Peer()
{
	leaveBG();
	mPeerCount--;
	mPeerSocket->shutdown(asio::ip::tcp::socket::shutdown_both);

	asio::error_code ec;
	mPeerSocket->close(ec);
	if (ec)
	{	LOG(Log::log(mSApair.toString(), " socket cannot close - ", ec.message());)		}
	delete mPeerSocket;
	DEBUG_LOG(Log::log(mSApair.toString(), " Peer Disconnected");)
}

std::string_view Peer::getCommandString()
{
	return mDataBuffer.getStringView();
}


void Peer::sendMessage(const Message* message, const std::string_view& bgTag)
{
	if (mPeerIsActive && mBgTag == bgTag)
	{
		mPeerSocket->async_send(asio::buffer(message->messageBuf.data(), message->messageBuf.size()),
			std::bind(&Peer::mSendMssgFuncFeedbk, this, std::placeholders::_1, std::placeholders::_2));
	}
}

void Peer::sendMessage(const Message* message)
{
	if (mPeerIsActive)
	{
		mPeerSocket->async_send(asio::buffer(message->messageBuf.data(), message->messageBuf.size()),
			std::bind(&Peer::mSendMssgFuncFeedbk, this, std::placeholders::_1, std::placeholders::_2));
	}
}


void Peer::mSendFuncFeedbk(const asio::error_code& ec, std::size_t size)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(mSApair.toString(), " Peer socket _sendData() failed", ec.message());)
		delete this;
	}
	else
		mPeerReceiveData();
}

void Peer::mSendMssgFuncFeedbk(const asio::error_code& ec, std::size_t size)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(mSApair.toString(), " Peer socket _sendMessage() failed", ec.message());)
		mPeerIsActive = false;
	}
}


void Peer::mSendPeerBufferData()
{
	mPeerSocket->async_send(mDataBuffer.getSendBuffer(), std::bind(&Peer::mSendFuncFeedbk,
		this, std::placeholders::_1, std::placeholders::_2));
}

void Peer::mPeerReceiveData()
{
	if (mPeerIsActive)
	{
		mPeerSocket->async_receive(mDataBuffer.getReadBuffer(), 0, std::bind(&Peer::mProcessData,
			this, std::placeholders::_1, std::placeholders::_2));
	}
	else
		delete this;
}

void Peer::mProcessData(const asio::error_code& ec, std::size_t dataSize)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(mSApair.toString(), " Peer socket _processData() failed ", ec.message());)
		delete this;
	}
	else
	{
		mDataBuffer.cookString(dataSize);
		CmdProcessor::processCommand(*this);

		if (mPeerIsActive)
			mSendPeerBufferData();
		else
			delete this;
	}
}


void Peer::disconnect()
{
	DEBUG_LOG(Log::log(mSApair.toString(), " Peer Disconnecting");)
	mPeerIsActive = false;
}

void Peer::listenTo(const std::string_view& bgID, const std::string_view& bgTag)
{
	std::string response;
	if (mIsInBG)
		response += "[R] " + CmdProcessor::RESP[(short)Response::IS_IN_BG];
	else
	{
		mBgID = bgID;
		mBgTag = bgTag;

		mBgPtr = BGcontroller::addToBG(this, mBgID);
		if (mBgPtr != nullptr)
		{
			mIsInBG = true;
			auto message = Message::makeAddMsg(mSApair);
			if (message != nullptr)
				mBgPtr->broadcast(this, message);

			response += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
			DEBUG_LOG(Log::log(mSApair.toString(), " Listening to Tag: ", mBgTag, " BG: ", mBgID);)
		}
		else
			response += "[R] " + CmdProcessor::RESP[(short)Response::WAIT_RETRY];
	}
	mDataBuffer = response;
}

void Peer::leaveBG()
{
	std::string response;
	if (mIsInBG)
	{
		DEBUG_LOG(Log::log(mSApair.toString(), " Peer leavig BG ", mBgID);)
		BGcontroller::removeFromBG(this, mBgID);
		auto message = Message::makeRemMsg(mSApair);
		if (message != nullptr)
			mBgPtr->broadcast(this, message);

		mIsInBG = false;
		mBgPtr = nullptr;
		response += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
	}
	else
		response += "[R] " + CmdProcessor::RESP[(short)Response::NOT_IN_BG];
	mDataBuffer = response;
}

void Peer::printPingInfo()
{
	std::string response;
	DEBUG_LOG(Log::log(mSApair.toString(), " Peer pinging");)
	response += "[R] " + mSApair.toString();
	mDataBuffer = response;
}

void Peer::respondWith(Response resp)
{
	std::string response;
	DEBUG_LOG(Log::log(mSApair.toString(), " Peer responding: ", CmdProcessor::RESP[(short)resp]);)
	response = "[R] " + CmdProcessor::RESP[(short)resp];
	mDataBuffer = response;
}

void Peer::broadcast(const std::string_view& messageStr)
{
	std::string response;
	if (mIsInBG)
	{
		auto message = Message::makeBrdMsg(mSApair, messageStr);
		if (message != nullptr)
			mBgPtr->broadcast(this, message);

		response += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::log(mSApair.toString(), " Peer broadcasting: ", messageStr);)
	}
	else
		response += "[R] " + CmdProcessor::RESP[(short)Response::NOT_IN_BG];
	mDataBuffer = response;
}

void Peer::broadcast(const std::string_view& messageStr, const std::string_view& bgTag)
{
	std::string response;
	if (mIsInBG)
	{
		auto message = Message::makeBrdMsg(mSApair, messageStr);
		if (message != nullptr)
			mBgPtr->broadcast(this, message, bgTag);

		response += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::log(mSApair.toString(), " Peer broadcasting: ", messageStr);)
	}
	else
		response += "[R] " + CmdProcessor::RESP[(short)Response::NOT_IN_BG];
	mDataBuffer = response;
}
