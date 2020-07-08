#include "ssl_ccm.h"
#include "cmd_processor.h"
#include "rtds_settings.h"
#include <functional>
#include "log.h"

SSLccm::SSLccm(SSLsocket* socketPtr)
{
	mPeerSocket = socketPtr;
	mIsAdmin = false;
	mPeerIsActive = true;

	DEBUG_LOG(Log::log("CCM Peer Connected");)
	mPeerReceiveData();
}

SSLccm::~SSLccm()
{
	delete mPeerSocket;
	DEBUG_LOG(Log::log("CCM Peer Disconnected");)
}


void SSLccm::mSendFuncFeedbk(const asio::error_code& ec)
{
	if (ec)
	{
		DEBUG_LOG(Log::log( "CCM Peer socket sendPeerBufferData() failed", ec.message());)
		delete this;
	}
	else
		mPeerReceiveData();
}


void SSLccm::mSendPeerBufferData()
{
	mPeerSocket->async_write_some(mDataBuffer.getSendBuffer(), 
		std::bind(&SSLccm::mSendFuncFeedbk, this, std::placeholders::_1));
}

void SSLccm::mPeerReceiveData()
{
	if (mPeerIsActive)
	{
		mPeerSocket->async_read_some(mDataBuffer.getReadBuffer(), 
			std::bind(&SSLccm::mProcessData, this, std::placeholders::_1, std::placeholders::_2));
	}
	else
		delete this;
}

void SSLccm::mProcessData(const asio::error_code& ec, std::size_t dataSize)
{
	if (ec)
	{
		DEBUG_LOG(Log::log("CCM Peer socket processData() failed ", ec.message());)
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
	DEBUG_LOG(Log::log("CCM Peer Disconnecting");)
	mPeerIsActive = false;
}

void SSLccm::status()
{
	std::string response = "[R]\t";
	if (mIsAdmin)
	{
		DEBUG_LOG(Log::log("CCM requesting status");)
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
		DEBUG_LOG(Log::log("CCM peer authenticated");)
	}
	else
		response += CmdProcessor::RESP[(short)Response::NOT_ALLOWED];
	response += "\n";
	mDataBuffer = response;
}

void SSLccm::respondWith(const Response resp)
{
	std::string response = "[R]\t";
	DEBUG_LOG(Log::log("CCM Peer responding: ", CmdProcessor::RESP[(short)resp]);)
	response += CmdProcessor::RESP[(short)resp];
	response += "\n";
	mDataBuffer = response;
}
