#ifndef UDP_PEER_H
#define UDP_PEER_H

#include <asio/ip/udp.hpp>
#include "advanced_buffer.h"
#include "message.h"
#include "common.h"

class UDPpeer
{
	static asio::ip::udp::socket* mPeerSocket;		// Reference to the UDP socket

	asio::ip::udp::endpoint mUDPep;					// UDP endpoint
	BGT mBgTag;										// Broadcast group Tag

/*******************************************************************************************
* @brief This callback function will be called after sending message
*
* @param[in] ec					Asio error code
********************************************************************************************/
	static void mSendMssgFuncFeedbk(const asio::error_code&);

public:
/*******************************************************************************************
* @brief Register the general UDP socket to which the data is to be send
*
* @param[in]			Pointer to the UDP socket
*
* @details
* UDP socket must be assigned before using any other functions
********************************************************************************************/
	static void registerUDPsocket(asio::ip::udp::socket* udpSock);
/*******************************************************************************************
* @brief Return the reference to the UDP endpoint
*
* @return				Return the UDP endpoint
*
* @details
* UDP endpoint must be assigned before using any other functions
********************************************************************************************/
	asio::ip::udp::endpoint& getRefToEp();
	const std::string getSApairString()const;

/*******************************************************************************************
* @brief Shedule a message to the peer system
*
* @param[in]			Message to be send
* @param[in]			Message Tag
*
* @details
* Only send the message if the tags are compatible.
********************************************************************************************/
	void sendMessage(const Message*, const std::string_view&);
	void sendMessage(const Message*);

/*******************************************************************************************
* @brief Print the source address pair info to the buffer
*
* @param[in]			Response Buffer
*
* @details
* Print Version, IP address and port number
********************************************************************************************/
	void printPingInfo(AdancedBuffer&)const;
/*******************************************************************************************
* @brief Send response to the peer
*
* @param[in]			Response
********************************************************************************************/
	void respondWith(const Response, AdancedBuffer&)const;
/*******************************************************************************************
* @brief Broadcast a message to the group
*
* @param[in]			Message
* @param[in]			Broadcast Group ID
* @param[in]			Broadcast Group Tag
********************************************************************************************/
	void broadcast(const std::string_view&, const std::string_view&, AdancedBuffer&)const;
	void broadcast(const std::string_view&, const std::string_view&, const std::string_view&, AdancedBuffer&)const;
};

#endif