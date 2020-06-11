#ifndef PEER_H
#define PEER_H

#include <asio.hpp>
#include "message.h"
#include "sapair.h"
#include "common.h"

class Peer
{		
	static std::atomic_int _peerCount;		// Keep the total count of peers

	ReceiveBuffer _dataBuffer;				// Buffer to which the commands are received
	asio::ip::tcp::socket* _peerSocket;		// Socket handling the data from peer system
	const SApair _saPair;					// Source address pair of this peer
	std::string _writeBuffer;				// Buffer from which the response will be send
	void* _bgPtr;							// Pointer to broadcast group

	std::mutex _resLock;					// Peer resource lock
	std::string _bgID;						// Broadcast group ID
	std::string _bgTag;						// Broadcast group Tag

	bool _peerIsActive;						// True if the peer socket is operational
	bool _isInBG;							// True if this peer is in Broadcast Group

/*******************************************************************************************
* @brief Shedule a send for writeBuffer contents to the peer system
*
* @details
* The callback function _sendFeedback() will be invoked after the data is send.
* The callback function will be called even if thier is a error in tcp connection.
********************************************************************************************/
	void _sendPeerBufferData();
/*******************************************************************************************
 * @brief Shedule handler funtion for peerSocket to receive the data in dataBuffer[]
 *
 * @details
 * The callback function _processData() will be invoked when their is new data in buffer.
 * The callback function will be called even if thier is a error in tcp connection.
 ********************************************************************************************/
	void _peerReceiveData();
/*******************************************************************************************
* @brief The callback function for getting (new data / socket error)
*
* @param[in] ec					Asio error code
* @param[in] size				Number of bytes received
*
* @details
* Append receved data with '\0' to make it string
* Pass the command to the command interpreter to process the command.
* Send back Response for the received command.
* If ec state a error in connection, this peer object will be deleted.
********************************************************************************************/
	void _processData(const asio::error_code&, std::size_t);
/*******************************************************************************************
* @brief This callback function will be called after sending data in the write buffer
*
* @param[in] ec					Asio error code
* @param[in] size				Number of bytes send
*
* @details
* If ec state a error in connection, this object will be deleted.
* If the connection is ok then, clear the write buffer and register for next receive.
********************************************************************************************/
	void _sendFuncFeedbk(const asio::error_code&, std::size_t);
/*******************************************************************************************
* @brief This callback function will be called after sending message
*
* @param[in] ec					Asio error code
* @param[in] size				Number of bytes send
*
* @details
* If ec state a error in connection, signal peer object to be deleted.
********************************************************************************************/
	void _sendMssgFuncFeedbk(const asio::error_code&, std::size_t);
/*******************************************************************************************
* @brief Shut down the socket and delete the peer object.
********************************************************************************************/
	void _terminatePeer();
/*******************************************************************************************
* @brief Close and delete peerSocket
*
* @details
* Leave the BG if in listening mode.
********************************************************************************************/
	~Peer();

public:
	std::string_view commandStr;			// A string view of the received command

/*******************************************************************************************
* @brief Create a Peer object with an accepted socketPtr*
*
* @param[in]			Pointer to the newly accepted socket
*
* @details
* Reserve buffer size and get the peer endpoint.
* Create a SourceAddressPair with the pointer to the socket. 
********************************************************************************************/
	Peer(asio::ip::tcp::socket*);
/*******************************************************************************************
* @brief Shedule a send for message to the peer system
*
* @param[in]			Message Tag
* @param[in]			Message to be send
*
* @details
* Only send the message if the tags are compatible.
* The callback function _sendMFeedback() will be invoked after the data is send.
* The callback function will be called even if thier is a error in tcp connection.
********************************************************************************************/
	void sendMessage(const std::string&, const Message* message);
/*******************************************************************************************
* @brief Disconnect the peer and delete the object
********************************************************************************************/
	void disconnect();
/*******************************************************************************************
* @brief Start listening to a brodcast group
*
* @param[in]			Broadcast Group ID
* @param[in]			Broadcast Group Tag
*
* @details
* Send WAIT_RETRY if peer failed to join the broadcast group
* Send SUCCESS if the joining was success 
********************************************************************************************/
	void listenTo(const std::string_view&, const std::string_view&);
/*******************************************************************************************
* @brief Leave the brodcast group
********************************************************************************************/
	void leaveBG();
/*******************************************************************************************
* @brief Change the Broadcast group tag 
*
* @param[in]			New Tag value
********************************************************************************************/
	void changeTagTo(const std::string_view&);
/*******************************************************************************************
* @brief Print the source address pair info to the write buffer
*
* @details
* Print Version, IP address and port number
********************************************************************************************/
	void printPingInfo();
/*******************************************************************************************
* @brief Send response to the peer
*
* @param[in]			Response
********************************************************************************************/
	void respondWith(const Response);
/*******************************************************************************************
* @brief Broadcast a message to the group 
*
* @param[in]			Message
********************************************************************************************/
	void broadcast(const std::string_view&);
};

#endif