#ifndef UDP_PEER_H
#define UDP_PEER_H

#include <asio/ip/udp.hpp>
#include "peer.h"

class UDPpeer : public Peer
{
	static asio::ip::udp::socket* mPeerSocket;		// Pointer to the UDP socket

	asio::ip::udp::endpoint mUDPep;					// UDP endpoint
/*******************************************************************************************
* @brief Send dataBuffer contents to the peer system
*
* @details
* The callback function _sendFeedback() will be invoked after the data is send.
* The callback function will be called even if thier is a error in tcp connection.
********************************************************************************************/
	void mSendPeerBufferData();

public:
/*******************************************************************************************
* @brief Register the general UDP socket
*
* @param[in]					UDP socket
********************************************************************************************/
	UDPpeer(asio::ip::udp::socket*);
/*******************************************************************************************
* @brief Return the reference to the UDP endpoint
*
* @return				Return the UDP endpoint
*
* @details
* UDP endpoint must be assigned before using any other functions
********************************************************************************************/
	asio::ip::udp::endpoint& getRefToEndpoint();

/*******************************************************************************************
* @brief Print the source address pair info to the buffer
*
* @param[in]			Response Buffer
*
* @details
* Print Version, IP address and port number
********************************************************************************************/
	void printPingInfo();
/*******************************************************************************************
* @brief Send response to the peer
*
* @param[in]			Response
* @param[in]			Response Buffer
********************************************************************************************/
	void respondWith(const Response);
/*******************************************************************************************
* @brief Broadcast a message to the group
*
* @param[in]			Message
* @param[in]			Broadcast Group ID
* @param[in]			Broadcast Group Tag
* @param[in]			Replay Buffer
********************************************************************************************/
	void broadcast(const std::string_view&, const std::string_view&, const std::string_view&);
};

#endif