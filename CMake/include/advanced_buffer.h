#ifndef ADV_BUFFER_H
#define ADV_BUFFER_H
#include "common.h"
#include <string>
#include <asio.hpp>

class AdancedBuffer
{
	std::array<char, RTDS_BUFF_SIZE + 1> mBuffer;			// Actual data buffer
	std::size_t mVirtualSize;								// Virtual size of the buffer
public:
	void operator =(const std::string&);
	void cookString(const size_t);
	asio::mutable_buffer getAsioBuffer();
	std::string_view getStringView();
};

#endif
