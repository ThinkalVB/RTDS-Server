#ifndef SSL_CCM_H
#define SSL_CCM_H

#include <asio/ip/tcp.hpp>
#include <asio/ssl.hpp>
#include "advanced_buffer.h"
typedef asio::ssl::stream<asio::ip::tcp::socket> SSLsocket;

class SSLccm
{
	AdancedBuffer mDataBuffer;				// Buffer to which the commands are received
	SSLsocket* mPeerSocket;					// Socket handling the data from peer system
	bool mPeerIsActive;						// True if the peer socket is operational
	bool mIsAdmin;							// True if have admin privileage

/*******************************************************************************************
* @brief Shedule handler funtion for peerSocket to receive the data in dataBuffer
*
* @details
* The callback function _processData() will be invoked when their is new data in buffer.
* The callback function will be called even if thier is a error in ssl connection.
********************************************************************************************/
	void mPeerReceiveData();
/*******************************************************************************************
* @brief The callback function for getting (new data / socket error)
*
* @param[in] ec					Asio error code
* @param[in] size				Number of bytes received
*
* @details
* Pass the command to the command interpreter to process the command.
* Send back Response for the received command.
* If ec state a error in connection, this peer object will be deleted.
********************************************************************************************/
	void mProcessData(const asio::error_code&, std::size_t);
/*******************************************************************************************
* @brief Shedule a send for writeBuffer contents to the peer system
*
* @details
* The callback function _sendFeedback() will be invoked after the data is send.
* The callback function will be called even if thier is a error in tcp connection.
********************************************************************************************/
	void mSendPeerBufferData();
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
* @brief Close and delete peerSocket
*
* @details
* Shutdown and delete peer socket.
********************************************************************************************/
	~SSLccm();

public:
/*******************************************************************************************
* @brief Create a Peer object with an accepted SSLsocket*
*
* @param[in]			Pointer to the newly accepted socket
********************************************************************************************/
	SSLccm(SSLsocket*);
/*******************************************************************************************
* @brief Return the string view of the received command
*
* @return				Return peer count
********************************************************************************************/
	std::string_view getCommandString();

/*******************************************************************************************
* @brief Terminate the RTDS server
********************************************************************************************/
	void abort();
/*******************************************************************************************
* @brief Disconnect the peer and delete the object
********************************************************************************************/
	void disconnect();
/*******************************************************************************************
* @brief Give status of the RTDS server
********************************************************************************************/
	void status();
/*******************************************************************************************
* @brief Login to RTDS
*
* @param[in]			Username
* @param[in]			Password
*
* @details
* Send NOT_ALLOWED if peer failed to authenticate
* Send SUCCESS if the joining was success
********************************************************************************************/
	void login(const std::string_view&, const std::string_view&);
/*******************************************************************************************
* @brief Send response to the peer
*
* @param[in]			Response
********************************************************************************************/
	void respondWith(const Response);
};
#endif