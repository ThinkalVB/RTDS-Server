#ifndef TCP_PEER_H
#define TCP_PEER_H

#include "stream_peer.h"

class TCPpeer : public StreamPeer
{		
	asio::ip::tcp::socket* mPeerSocket;		// Socket handling the data from peer system

/*******************************************************************************************
* @brief Shedule a send for dataBuffer contents to the peer system
*
* @details
* The callback function _sendFeedback() will be invoked after the data is send.
* The callback function will be called even if thier is a error in tcp connection.
********************************************************************************************/
	void mSendPeerBufferData();
/*******************************************************************************************
 * @brief Shedule handler funtion for peerSocket to receive the data in Data Buffer
 *
 * @details
 * The callback function _processData() will be invoked when their is new data in buffer.
 * The callback function will be called even if thier is a error in tcp connection.
 ********************************************************************************************/
	void mPeerReceiveData();
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
	void mProcessData(const asio::error_code&, std::size_t);
/*******************************************************************************************
* @brief This callback function will be called after sending data in the write buffer
*
* @param[in] ec					Asio error code
*
* @details
* If ec state a error in connection, this object will be deleted.
* If the connection is ok then, clear the write buffer and register for next receive.
********************************************************************************************/
	void mSendFuncFeedbk(const asio::error_code&);
/*******************************************************************************************
* @brief This callback function will be called after sending message
*
* @param[in] ec			Asio error code
*
* @details
* If ec state a error in connection, signal peer object to be deleted.
********************************************************************************************/
	void mSendMssgFuncFeedbk(const asio::error_code&);
/*******************************************************************************************
* @brief Close and delete peerSocket
*
* @details
* Leave the BG if in listening mode.
* Decrement peer count. Shutdown and delete peer socket.
********************************************************************************************/
	~TCPpeer();

public:
/*******************************************************************************************
* @brief Create a Peer object with an accepted socketPtr*
*
* @param[in]			Pointer to the newly accepted socket
*
* @details
* Create a SourceAddressPair with the pointer to the socket. 
********************************************************************************************/
	TCPpeer(asio::ip::tcp::socket*);
/*******************************************************************************************
* @brief Shedule a send for message to the peer system
*
* @param[in]			Message to be send
* @param[in]			Message Tag
*
* @details
* Only send the message if the tags are compatible.
* The callback function _sendMFeedback() will be invoked after the data is send.
* The callback function will be called even if thier is a error in tcp connection.
********************************************************************************************/
	void sendMessage(const Message*);
};

#endif