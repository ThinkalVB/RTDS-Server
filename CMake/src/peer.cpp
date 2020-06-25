#include "peer.h"
#include "cmd_processor.h"
#include "bg_controller.h"
#include "log.h"

std::atomic_int Peer::mPeerCount = 0;

Peer::Peer(asio::ip::tcp::socket* socketPtr) : mSApair(socketPtr)
{
	mPeerSocket = socketPtr;
	mWriteBuffer.reserve(RTDS_BUFF_SIZE);

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
	{
		mWriteBuffer.clear();
		mPeerReceiveData();
	}
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
	mPeerSocket->async_send(asio::buffer(mWriteBuffer.data(), mWriteBuffer.size()), std::bind(&Peer::mSendFuncFeedbk,
		this, std::placeholders::_1, std::placeholders::_2));
}

void Peer::mPeerReceiveData()
{
	if (mPeerIsActive)
	{
		mPeerSocket->async_receive(mDataBuffer.getAsioBuffer(), 0, std::bind(&Peer::mProcessData,
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
		commandStr = mDataBuffer.getStringView();
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
	if (mIsInBG)
		mWriteBuffer += "[R] " + CmdProcessor::RESP[(short)Response::IS_IN_BG];
	else
	{
		mBgID = bgID;
		mBgTag = bgTag;

		mBgPtr = BGcontroller::addToBG(this, mBgID);
		if (mBgPtr != nullptr)
		{
			mIsInBG = true;
			auto message = Message::makeAddMsg(mSApair, mBgTag);
			if (message != nullptr)
				mBgPtr->broadcast(this, message);

			mWriteBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
			DEBUG_LOG(Log::log(mSApair.toString(), " Listening to Tag: ", mBgTag, " BG: ", mBgID);)
		}
		else
			mWriteBuffer += "[R] " + CmdProcessor::RESP[(short)Response::WAIT_RETRY];
	}
}

void Peer::leaveBG()
{
	if (mIsInBG)
	{
		DEBUG_LOG(Log::log(mSApair.toString(), " Peer leavig BG ", mBgID);)
		BGcontroller::removeFromBG(this, mBgID);
		auto message = Message::makeRemMsg(mSApair, mBgTag);
		if (message != nullptr)
			mBgPtr->broadcast(this, message);

		mIsInBG = false;
		mBgPtr = nullptr;
		mWriteBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
	}
	else
		mWriteBuffer += "[R] " + CmdProcessor::RESP[(short)Response::NOT_IN_BG];
}

void Peer::printPingInfo()
{
	DEBUG_LOG(Log::log(mSApair.toString(), " Peer pinging");)
	mWriteBuffer += "[R] " + mSApair.toString();
}

void Peer::respondWith(Response response)
{
	DEBUG_LOG(Log::log(mSApair.toString(), " Peer responding: ", CmdProcessor::RESP[(short)response]);)
	mWriteBuffer += "[R] " + CmdProcessor::RESP[(short)response];
}

void Peer::broadcast(const std::string_view& messageStr)
{
	if (mIsInBG)
	{
		auto message = Message::makeBrdMsg(mSApair, messageStr);
		if (message != nullptr)
			mBgPtr->broadcast(this, message);

		mWriteBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::log(mSApair.toString(), " Peer broadcasting: ", messageStr);)
	}
	else
		mWriteBuffer += "[R] " + CmdProcessor::RESP[(short)Response::NOT_IN_BG];
}

void Peer::broadcast(const std::string_view& messageStr, const std::string_view& bgTag)
{
	if (mIsInBG)
	{
		auto message = Message::makeBrdMsg(mSApair, messageStr);
		if (message != nullptr)
			mBgPtr->broadcast(this, message, bgTag);

		mWriteBuffer += "[R] " + CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::log(mSApair.toString(), " Peer broadcasting: ", messageStr);)
	}
	else
		mWriteBuffer += "[R] " + CmdProcessor::RESP[(short)Response::NOT_IN_BG];
}
