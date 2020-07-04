#include "ssl_ccm.h"
#include "cmd_processor.h"
#include "rtds_settings.h"
#include <functional>
#include "log.h"
#include "rtds.h"

std::atomic_int SSLccm::mPeerCount = 0;

SSLccm::SSLccm(SSLsocket* socketPtr)
{
	mPeerSocket = socketPtr;
	mIsAdmin = false;
	mPeerIsActive = true;

	DEBUG_LOG(Log::log("SSL Peer Connected");)
	mPeerCount++;
	mPeerReceiveData();
}

int SSLccm::peerCount()
{
	return mPeerCount;
}

SSLccm::~SSLccm()
{
	mPeerCount--;
	asio::error_code ec;
	mPeerSocket->shutdown(ec);
	if (ec)
	{	DEBUG_LOG(Log::log("SSL socket cannot shutdown - ", ec.message());)	}
	mPeerSocket->lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both, ec);
	if (ec)
	{	DEBUG_LOG(Log::log("SSL lowest layer cannot shutdown - ", ec.message());)	}
	mPeerSocket->lowest_layer().close(ec);
	if (ec)
	{	DEBUG_LOG(Log::log("SSL socket cannot close - ", ec.message());)	}

	delete mPeerSocket;
	DEBUG_LOG(Log::log("SSL Peer Disconnected");)
}

std::string_view SSLccm::getCommandString()
{
	return mDataBuffer.getStringView();
}


void SSLccm::mPeerReceiveData()
{
	if (mPeerIsActive)
	{
		mPeerSocket->async_read_some(mDataBuffer.getReadBuffer(), std::bind(&SSLccm::mProcessData,
			this, std::placeholders::_1, std::placeholders::_2));
	}
	else
		delete this;
}

void SSLccm::mProcessData(const asio::error_code& ec, std::size_t dataSize)
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

void SSLccm::mSendPeerBufferData()
{
	mPeerSocket->async_write_some(mDataBuffer.getSendBuffer(), std::bind(&SSLccm::mSendFuncFeedbk,
		this, std::placeholders::_1));
}

void SSLccm::mSendFuncFeedbk(const asio::error_code& ec)
{
	if (ec)
	{
		DEBUG_LOG(Log::log("SSL Peer socket _sendMessage() failed", ec.message());)
			mPeerIsActive = false;
	}
	else
		mPeerReceiveData();
}


void SSLccm::abort()
{
	std::string response = "[R]\t";
	if (mIsAdmin)
	{
		DEBUG_LOG(Log::log("RTDS aborted");)
		SIGNAL_ABORT
	}
	else
		response += CmdProcessor::RESP[(short)Response::BAD_COMMAND];
	response += "\n";
	mDataBuffer = response;
}

void SSLccm::disconnect()
{
	DEBUG_LOG(Log::log("SSL Peer Disconnecting");)
	mPeerIsActive = false;
}

void SSLccm::status()
{
	std::string response = "[R]\t";
	if (mIsAdmin)
	{
		DEBUG_LOG(Log::log("RTDS requesting status");)
		response += Settings::generateStatus();
	}
	else
		response += CmdProcessor::RESP[(short)Response::NOT_ALLOWED];
	response += "\n";
	mDataBuffer = response;

}

void SSLccm::login(const std::string_view& usr, const std::string_view& pass)
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

void SSLccm::respondWith(const Response resp)
{
	std::string response = "[R]\t";
	DEBUG_LOG(Log::log("SSL Peer responding: ", CmdProcessor::RESP[(short)resp]);)
	response += CmdProcessor::RESP[(short)resp];
	response += "\n";
	mDataBuffer = response;
}
