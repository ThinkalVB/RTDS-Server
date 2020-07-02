#include "ssl_peer.h"
#include "cmd_processor.h"
#include "rtds_ccm.h"
#include <functional>
#include "log.h"
#include "rtds.h"

SSLpeer::SSLpeer(SSLsocket* socketPtr)
{
	mPeerSocket = socketPtr;
	mIsAdmin = false;
	mPeerIsActive = true;

	DEBUG_LOG(Log::log("SSL Peer Connected");)
	mPeerReceiveData();
}

std::string_view SSLpeer::getCommandString()
{
	return mDataBuffer.getStringView();
}

void SSLpeer::abort()
{
	std::string response = "[R]\t";
	if (mIsAdmin)
	{
		DEBUG_LOG(Log::log("RTDS aborted");)
		response += CmdProcessor::RESP[(short)Response::SUCCESS];
		SIGNAL_ABORT
	}
	else
		response += CmdProcessor::RESP[(short)Response::BAD_COMMAND];
	response += "\n";
	mDataBuffer = response;
}

void SSLpeer::disconnect()
{
	DEBUG_LOG(Log::log("SSL Peer Disconnecting");)
	mPeerIsActive = false;
}

void SSLpeer::login(const std::string_view& usr, const std::string_view& pass)
{
	std::string response = "[R]\t";
	if (usr == ROOT_USRN && pass == ROOT_PASS)
	{
		mIsAdmin = true;
		response += CmdProcessor::RESP[(short)Response::SUCCESS];
		DEBUG_LOG(Log::log("SSL peer authenticated");)
	}
	else
		response += CmdProcessor::RESP[(short)Response::NOT_ALLOWED];
	response += "\n";
	mDataBuffer = response;
}

void SSLpeer::respondWith(const Response resp)
{
	std::string response = "[R]\t";
	DEBUG_LOG(Log::log("SSL Peer responding: ", CmdProcessor::RESP[(short)resp]);)
	response += CmdProcessor::RESP[(short)resp];
	response += "\n";
	mDataBuffer = response;
}

void SSLpeer::mPeerReceiveData()
{
	if (mPeerIsActive)
	{
		mPeerSocket->async_read_some(mDataBuffer.getReadBuffer(), std::bind(&SSLpeer::mProcessData,
			this, std::placeholders::_1, std::placeholders::_2));
	}
	else
		delete this;
}

void SSLpeer::mProcessData(const asio::error_code& ec, std::size_t dataSize)
{
	if (ec)
	{
		DEBUG_LOG(Log::log("SSL Peer socket _processData() failed ", ec.message());)
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

void SSLpeer::mSendPeerBufferData()
{
	mPeerSocket->async_write_some(mDataBuffer.getSendBuffer(), std::bind(&SSLpeer::mSendFuncFeedbk,
		this, std::placeholders::_1));
}

void SSLpeer::mSendFuncFeedbk(const asio::error_code& ec)
{
	if (ec)
	{
		DEBUG_LOG(Log::log("SSL Peer socket _sendMessage() failed", ec.message());)
		mPeerIsActive = false;
	}
	else
		mPeerReceiveData();
}

SSLpeer::~SSLpeer()
{
	asio::error_code ec;
	mPeerSocket->shutdown(ec);
	if (ec)
	{	LOG(Log::log("SSL socket cannot shutdown - ", ec.message());)		}
	mPeerSocket->lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both, ec);
		if (ec)
	{	LOG(Log::log("SSL lowest laayer cannot shutdown - ", ec.message());)	}
	mPeerSocket->lowest_layer().close(ec);
	if (ec)
	{	LOG(Log::log("SSL socket cannot close - ", ec.message());)		}

	delete mPeerSocket;
	DEBUG_LOG(Log::log("SSL Peer Disconnected");)
}
