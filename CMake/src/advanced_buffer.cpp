#include "advanced_buffer.h"

void AdancedBuffer::operator=(const std::string& responseStr)
{
	memcpy(mBuffer.data(), responseStr.data(), responseStr.length());
	mVirtualSize = responseStr.length();
}

bool AdancedBuffer::cookString(const size_t noOfStrBytes)
{
	mVirtualSize = noOfStrBytes - 1;
	if (mBuffer[mVirtualSize] == '\n')
		return true;
	else
		return false;
}

asio::mutable_buffer AdancedBuffer::getReadBuffer()
{
	return asio::mutable_buffer(mBuffer.data(), RTDS_BUFF_SIZE);
}

asio::mutable_buffer AdancedBuffer::getSendBuffer()
{
	return asio::mutable_buffer(mBuffer.data(), mVirtualSize);
}

std::string_view AdancedBuffer::getStringView() const
{
	std::string_view strView((char*)mBuffer.data(), mVirtualSize);
	return strView;
}
