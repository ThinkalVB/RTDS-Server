#include "advanced_buffer.h"

void AdancedBuffer::operator=(const std::string& responseStr)
{
	strcpy_s(m_buffer.data(), RTDS_BUFF_SIZE + 1, responseStr.c_str());
	m_virtualSize = responseStr.length();
}

void AdancedBuffer::cookString(const size_t noOfStrBytes)
{
	m_buffer[noOfStrBytes] = '\0';
	m_virtualSize = noOfStrBytes;
}

asio::mutable_buffer AdancedBuffer::getAsioBuffer()
{
	return asio::mutable_buffer(m_buffer.data(), m_virtualSize);
}
