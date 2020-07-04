#ifndef PEER_H
#define PEER_H

#include "advanced_buffer.h"

class Peer
{
protected:
	AdancedBuffer mDataBuffer;				// Buffer to which the commands are received

public:
/*******************************************************************************************
* @brief Return the string view of the received command
*
* @return				Return peer count
********************************************************************************************/
	std::string_view getCommandString() const;
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
* @brief Prepare the buffer to be converted to a string view
*
* @param[in]		Size of the received data (in bytes)
* @return			True if ends with newline
*
* @details
* Check for newline char at the end of the received data, else return false
********************************************************************************************/
	bool cookString(std::size_t);
};

#endif