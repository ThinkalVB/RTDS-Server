#ifndef ADV_BUFFER_H
#define ADV_BUFFER_H
#include <asio/buffer.hpp>
#include <string>
#include "common.h"

class AdancedBuffer
{
	std::array<char, RTDS_BUFF_SIZE> mBuffer;				// Actual data buffer
	std::size_t mVirtualSize;								// Virtual size of the buffer
public:
/*******************************************************************************************
* @brief Copy a string to the buffer and set the virtual size for asio buffer
*
* @param[in]		Response string to the command
********************************************************************************************/
	void operator =(const std::string&);
/*******************************************************************************************
* @brief Prepare the buffer to be converted to a string view
*
* @param[in]		Size of the received data (in bytes)
*
* @details
* Append a null char at the end of the received data
********************************************************************************************/
	void cookString(const size_t);
/*******************************************************************************************
* @brief Get read buffer (Get the entire buffer for reading from peer)
*
* @return				Asio buffer of the underlying char array
********************************************************************************************/
	asio::mutable_buffer getReadBuffer();
/*******************************************************************************************
* @brief Get send buffer (Get the string buffer for sending data)
*
* @return				Asio buffer of the underlying char array
********************************************************************************************/
	asio::mutable_buffer getSendBuffer();
/*******************************************************************************************
* @brief Get the string view of the last cooked data or assigned string
*
* @return				String view of the data in buffer
*
* @details
* Must cook before requesting a string_view.
********************************************************************************************/
	std::string_view getStringView() const;
};

#endif
