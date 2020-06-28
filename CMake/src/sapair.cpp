#include "sapair.h"
#include "common.h"

SApair::SApair(asio::ip::tcp::socket* tcpSocket)
{
	auto remoteEp = tcpSocket->remote_endpoint();
	mIpAddr = remoteEp.address();
	mPortNumber = remoteEp.port();

	if (mIpAddr.is_v4())
	{
		mSaPairStr += STR_V4;
		auto _ipAddr4 = mIpAddr.to_v4();
		mSaPairStr += "\t" + _ipAddr4.to_string();
	}
	else
	{
		mSaPairStr += STR_V6;
		auto _ipAddr6 = mIpAddr.to_v6();
		if (_ipAddr6.is_v4_mapped())
			mSaPairStr += "\t" + _ipAddr6.to_v4().to_string();
		else
			mSaPairStr += "\t" + _ipAddr6.to_string();
	}
	mSaPairStr += "\t" + std::to_string(mPortNumber);
}

std::string SApair::toString() const
{
	return mSaPairStr;
}

bool SApair::operator==(const SApair& saPair) const
{
	if (mIpAddr == saPair.mIpAddr && mPortNumber == saPair.mPortNumber)
		return true;
	else
		return false;
}
