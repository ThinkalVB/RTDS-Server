#include "tcp_peer.h"
#include <functional>
#include "cmd_processor.h"
#include "bg_controller.h"
#include "log.h"

TCPpeer::TCPpeer(asio::ip::tcp::socket* socketPtr)
{
	mSApair = CmdProcessor::getSAPstring(socketPtr->remote_endpoint());
	mPeerSocket = socketPtr;

	DEBUG_LOG(Log::log(mSApair," TCP Peer Connected");)
	mPeerReceiveData();
}

TCPpeer::~TCPpeer()
{
	leaveBG();
	mPeerSocket->shutdown(asio::ip::tcp::socket::shutdown_both);

	asio::error_code ec;
	mPeerSocket->close(ec);
	if (ec)
	{	LOG(Log::log(mSApair, " socket cannot close - ", ec.message());)		}
	delete mPeerSocket;
	DEBUG_LOG(Log::log(mSApair, " TCP Peer Disconnected");)
}

void TCPpeer::mSendFuncFeedbk(const asio::error_code& ec)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(mSApair, " Peer socket _sendData() failed", ec.message());)
		delete this;
	}
	else
		mPeerReceiveData();
}

void TCPpeer::mSendMssgFuncFeedbk(const asio::error_code& ec)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(mSApair, " Peer socket _sendMessage() failed", ec.message());)
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
		DEBUG_LOG(Log::log(mSApair, " Peer socket _processData() failed ", ec.message());)
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

void TCPpeer::sendMessage(const Message* message)
{
	std::shared_lock<std::shared_mutex> readLock(mPeerResourceMtx);
	if (mPeerIsActive && (message->recverTag == ALL_TAG || message->recverTag == mBgTag))
	{
		mPeerSocket->async_send(asio::buffer(message->messageBuf.data(), message->messageBuf.size()),
			std::bind(&TCPpeer::mSendMssgFuncFeedbk, this, std::placeholders::_1));
	}
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

void TCPpeer::leaveBG()
{
	std::string response = "[R]\t";
	if (mIsInBG)
	{
		DEBUG_LOG(Log::log(mSApair, " Peer leavig BG ", mBgID);)
		BGcontroller::removeFromBG(this, mBgID);

		if (mPeerMode == PeerMode::LISTEN)
		{
			auto message = Message::makeRemMsg(mSApair, mBgTag, mBgTag, PeerType::TCP);
			if (message != nullptr)
				mBgPtr->broadcast(this, message);
			else
			{	LOG(Log::log(mSApair, " Failed to create leaving message!");)	}
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

void TCPpeer::broadcastTo(const std::string_view& messageStr, const std::string_view& bgTag)
{
	std::string response = "[R]\t";
	if (mIsInBG)
	{
		auto message = Message::makeBrdMsg(mSApair, messageStr, mBgTag, bgTag, PeerType::TCP);
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
