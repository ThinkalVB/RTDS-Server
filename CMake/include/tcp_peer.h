#ifndef TCP_PEER_H
#define TCP_PEER_H

#include <asio/ip/tcp.hpp>
#include <shared_mutex>
#include "message.h"
#include "common.h"


#include "advanced_buffer.h"

#include "stream_peer.h"

class BGroup;
class TCPpeer
{		
	AdancedBuffer mDataBuffer;				// Buffer to which the commands are received
	asio::ip::tcp::socket* mPeerSocket;		// Socket handling the data from peer system
	std::shared_mutex mPeerResourceMtx;		// Mutex for locking the shared resources

	PeerMode mPeerMode;						// Hearing mode of the peer
	BGroup* mBgPtr;							// Pointer to broadcast group
	BGID mBgID;								// Broadcast group ID
	BGT mBgTag;								// Broadcast group Tag
	SAP mSApair;							// Source Address Pair string

	bool mPeerIsActive;						// True if the peer socket is operational
	bool mIsInBG;							// True if this peer is in Broadcast Group

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
* @param[in] ec					Asio error code
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
* @brief Return the number of peers
*
* @return				Return peer count
********************************************************************************************/
	static int peerCount();
/*******************************************************************************************
* @brief Return the string view of the received command
*
* @return				Return peer count
********************************************************************************************/
	std::string_view getCommandString();
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
* All group members are notified
********************************************************************************************/
	void listenTo(const std::string_view&, const std::string_view&);
/*******************************************************************************************
* @brief Start hearing to a brodcast group
*
* @param[in]			Broadcast Group ID
* @param[in]			Broadcast Group Tag
*
* @details
* Send WAIT_RETRY if peer failed to join the broadcast group
* Send SUCCESS if the joining was success
********************************************************************************************/
	void hearTo(const std::string_view&, const std::string_view&);
/*******************************************************************************************
* @brief Leave the brodcast group
********************************************************************************************/
	void leaveBG();
/*******************************************************************************************
* @brief Change the tag if participating in a group
*
* @param[in]			Broadcast Group Tag
*
* @details
* Send NOT_IN_BG if peer is not in a broadcast group
* Send SUCCESS if the changing was success
********************************************************************************************/
	void changeTag(const std::string_view&);
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
* @param[in]			Broadcast Group Tag
********************************************************************************************/
	void broadcastTo(const std::string_view&, const std::string_view&);
};

#endif