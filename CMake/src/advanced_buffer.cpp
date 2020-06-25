#include "advanced_buffer.h"

void AdancedBuffer::operator=(const std::string& responseStr)
{
	strcpy_s(mBuffer.data(), RTDS_BUFF_SIZE + 1, responseStr.c_str());
	mVirtualSize = responseStr.length();
}

void AdancedBuffer::cookString(const size_t noOfStrBytes)
{
	mBuffer[noOfStrBytes] = '\0';
	mVirtualSize = noOfStrBytes;
}

asio::mutable_buffer AdancedBuffer::getAsioBuffer()
{
	return asio::mutable_buffer(mBuffer.data(), RTDS_BUFF_SIZE);
}

std::string_view AdancedBuffer::getStringView()
{
	std::string_view strView;
	strView = (char*)mBuffer.data();
	return strView;
}
