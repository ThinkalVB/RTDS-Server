#pragma once
#include <boost/asio.hpp>
#include "sp_entry.h"
#include <list>

using namespace boost;
constexpr short RTDS_BUFF_SIZE = 300;

class Peer
{
	static short peerCount;						//!< Keep the total count of peers
	asio::ip::tcp::socket* peerSocket;			//!< Socket handling the data from peer system
	asio::ip::tcp::endpoint remoteEp;			//!< Endpoint of the peerSocket with info on peer system
	Entry peerEntry;							//!< Union DS that store the SourcePair entry(v4/v6) pointers

	char dataBuffer[RTDS_BUFF_SIZE];			//!< Buffer to which the commands are received
	std::string writeBuffer;					//!< Buffer from which the response will be send

/*******************************************************************************************
 * @brief Shedule handler funtion for peerSocket to receive the data in dataBuffer[]
 *
 * @details
 * The callback function _processData() will be invoked when their is new data in buffer.
 * The callback function will be called even if thier is a error in tcp connection.
 ********************************************************************************************/
	void _peerReceiveData();
/*******************************************************************************************
* @brief Shedule a send for writeBuffer contents to the peer system
*
* @details
* The callback function _sendData() will be invoked after the data is send.
* The callback function will be called even if thier is a error in tcp connection.
********************************************************************************************/
	void _sendPeerData();
/*******************************************************************************************
* @brief The callback function for getting (new data / socket error)
*
* @param[in] ec					Asio error code
* @param[in] size				Number of bytes received
*
* @details
* Check if the data is a ';' terminated string, then append it with '\0' to make it string
* Pass the command to the command interpreter to process the command.
* If the received command is not ';' terminated then send back Response::BAD_COMMAND.
* If ec state a error in connection, this peer object will be deleted.
* This function won't register the next read - it have to be done by _sendData() callback fn.
********************************************************************************************/
	void _processData(const boost::system::error_code&, std::size_t);
/*******************************************************************************************
* @brief The callback function will be called after sending data in the write buffer
*
* @param[in] ec					Asio error code
* @param[in] size				Number of bytes send
*
* @details
* If ec state a error in connection, this peer object will be deleted.
* If the connection is ok then, clear the write buffer and register for next receive.
********************************************************************************************/
	void _sendData(const boost::system::error_code&, std::size_t);

public:
/*******************************************************************************************
* @brief Create a Peer object with an accepted socketPtr*
*
* @param[in] socketPtr			Pointer to the newly accepted socket
*
* @details
* Reserver buffer size and get the peer endpoint.
* Create a SourcePort Entry peerEntry with details associated with this peer.
* Set the flag isWithPeer to true so that Directory won't delete this entry.
********************************************************************************************/
	Peer(asio::ip::tcp::socket*);
/*******************************************************************************************
* @brief Close and delete peerSocket
*
* @details
* Set the flag isWithPeer to false so that Directory may delete this entry.
********************************************************************************************/
	~Peer();
/*******************************************************************************************
* @brief Get the total number of peers
*
* @return						Total number of peers
*
* @details
* Gives total number of open sockets listening to a remote system.
********************************************************************************************/
	static short getPeerCount();
	friend class CmdInterpreter;
};