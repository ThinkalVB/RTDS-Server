#include "tcp_peer.h"
#include <functional>
#include "cmd_processor.h"
#include "log.h"

TCPpeer::TCPpeer(asio::ip::tcp::socket* socketPtr)
{
	mPeerSocket = socketPtr;
	mSApair = CmdProcessor::getSAPstring(socketPtr->remote_endpoint());
	mPeerType = PeerType::TCP;

	DEBUG_LOG(Log::log(mSApair," TCP Peer Connected");)
	mPeerReceiveData();
}

TCPpeer::~TCPpeer()
{
	leaveBG();
	delete mPeerSocket;
	DEBUG_LOG(Log::log(mSApair, " TCP Peer socket Disconnected");)
}


void TCPpeer::mSendFuncFeedbk(const asio::error_code& ec)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(mSApair, " Peer socket sendPeerBufferData() failed", ec.message());)
		delete this;
	}
	else
		mPeerReceiveData();
}

void TCPpeer::mSendMssgFuncFeedbk(const asio::error_code& ec)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(mSApair, " Peer socket sendMessage() failed", ec.message());)
		mPeerIsActive = false;
	}
}


void TCPpeer::mSendPeerBufferData()
{
	mPeerSocket->async_send(mDataBuffer.getSendBuffer(), 
		std::bind(&TCPpeer::mSendFuncFeedbk, this, std::placeholders::_1));
}

void TCPpeer::mPeerReceiveData()
{
	if (mPeerIsActive)
	{
		mPeerSocket->async_receive(mDataBuffer.getReadBuffer(), 0, 
			std::bind(&TCPpeer::mProcessData, this, std::placeholders::_1, std::placeholders::_2));
	}
	else
		delete this;
}

void TCPpeer::mProcessData(const asio::error_code& ec, std::size_t dataSize)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(mSApair, " Peer socket processData() failed ", ec.message());)
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
		mPeerSocket->async_send(message->asioBuffer, 
			std::bind(&TCPpeer::mSendMssgFuncFeedbk, this, std::placeholders::_1));
	}
}
