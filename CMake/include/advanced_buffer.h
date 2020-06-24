#ifndef ADV_BUFFER_H
#define ADV_BUFFER_H
#include "common.h"
#include <string>
#include <asio.hpp>

class AdancedBuffer
{
	std::array<char, RTDS_BUFF_SIZE + 1> m_buffer;		// Actual data buffer
	std::size_t m_virtualSize;							// Virtual size of the buffer
public:
	void operator =(const std::string&);
	void cookString(const size_t);
	asio::mutable_buffer getAsioBuffer();
};

#endif
