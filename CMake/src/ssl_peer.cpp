#include "ssl_peer.h"
#include <functional>
#include "cmd_processor.h"
#include "bg_controller.h"
#include "log.h"

SSLpeer::SSLpeer(SSLsocket* socketPtr)
{
	mPeerSocket = socketPtr;
	mSApair = CmdProcessor::getSAPstring(socketPtr->lowest_layer().remote_endpoint());
	mPeerType = PeerType::SSL;

	DEBUG_LOG(Log::log(mSApair," SSL Peer Connected");)
	mPeerReceiveData();
}

SSLpeer::~SSLpeer()
{
	leaveBG();
	delete mPeerSocket;
	DEBUG_LOG(Log::log(mSApair, " SSL Peer socket Disconnected");)
}


void SSLpeer::mSendFuncFeedbk(const asio::error_code& ec)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(mSApair, " Peer socket sendPeerBufferData() failed", ec.message());)
		delete this;
	}
	else
		mPeerReceiveData();
}

void SSLpeer::mSendMssgFuncFeedbk(const asio::error_code& ec)
{
	if (ec)
	{
		DEBUG_LOG(Log::log(mSApair, " Peer socket sendMessage() failed", ec.message());)
		mPeerIsActive = false;
	}
}


void SSLpeer::mSendPeerBufferData()
{
	mPeerSocket->async_write_some(mDataBuffer.getSendBuffer(), 
		std::bind(&SSLpeer::mSendFuncFeedbk, this, std::placeholders::_1));
}

void SSLpeer::mPeerReceiveData()
{
	if (mPeerIsActive)
	{
		mPeerSocket->async_read_some(mDataBuffer.getReadBuffer(), 
			std::bind(&SSLpeer::mProcessData, this, std::placeholders::_1, std::placeholders::_2));
	}
	else
		delete this;
}

void SSLpeer::mProcessData(const asio::error_code& ec, std::size_t dataSize)
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

void SSLpeer::sendMessage(const Message* message)
{
	std::shared_lock<std::shared_mutex> readLock(mPeerResourceMtx);
	if (mPeerIsActive && (message->recverTag == ALL_TAG || message->recverTag == mBgTag))
	{
		mPeerSocket->async_write_some(message->asioBuffer, 
			std::bind(&SSLpeer::mSendMssgFuncFeedbk, this, std::placeholders::_1));
	}
}
