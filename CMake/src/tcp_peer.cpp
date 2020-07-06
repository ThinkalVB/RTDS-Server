#include "tcp_peer.h"
#include <functional>
#include "cmd_processor.h"
#include "bg_controller.h"
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
		mPeerSocket->async_send(message->asioBuffer, std::bind(&TCPpeer::mSendMssgFuncFeedbk, this, std::placeholders::_1));
	}
}
