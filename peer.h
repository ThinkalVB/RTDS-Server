#ifndef PEER_H
#define PEER_H

#include <boost/asio.hpp>
#include "cmd_element.h"
#include "notification.h"
#include "spaddress.h"

using namespace boost;

class Peer
{
	static short _peerCount;							//!< Keep the total count of peers
	static std::vector<Peer*> _mirrorGroup;				//!< Mirroring group
	static std::mutex _mgAccessLock;					//!< Lock this mutex before accessing mirror group

	ReceiveBuffer _dataBuffer;							//!< Buffer to which the commands are received
	std::mutex _resourceMtx;							//!< Mutex to prevent resource expiry while processing

	asio::ip::tcp::socket* _peerSocket;					//!< Socket handling the data from peer system
	bool _peerIsActive;									//!< True if the peer socket is operational
	const asio::ip::tcp::endpoint _remoteEp;			//!< Endpoint of the peerSocket with info on peer system
	std::atomic<Policy> _mirrorPolicy;					//!< Mirroring policy of the peer
	bool _isMirroring = false;							//!< True if this peer is in mirroring mode

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
/*******************************************************************************************
* @brief The callback function will be called after the notification is send
*
* @param[in] ec					Asio error code
* @param[in] size				Number of bytes send
*
* @details
* If ec state a error in connection, this peer object will be deleted.
********************************************************************************************/
	void _sendNotification(const boost::system::error_code&, std::size_t);

public:
	const SPaddress spAddress;							//!< Source pair address of the entry.
	CommandElement cmdElement;							//!< String view Array of command elements
	std::string writeBuffer;							//!< Buffer from which the response will be send

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
* @brief Shedule a send for writeBuffer contents to the peer system
*
* @details
* The callback function _sendData() will be invoked after the data is send.
* If the write buffer is empty then a bad_command response will be send.
* The callback function will be called even if thier is a error in tcp connection.
********************************************************************************************/
	void sendPeerData();
/*******************************************************************************************
* @brief Get the total number of peers
*
* @return						Total number of peers
*
* @details
* Gives total number of open sockets listening to a remote system.
********************************************************************************************/
static short getPeerCount();
/*******************************************************************************************
* @brief Shut down the socket and delete the peer object.
********************************************************************************************/
void terminatePeer();
/*******************************************************************************************
* @brief Add this peer to mirroring group
*
* @details
* Set the flag isMirroring to true and add to the mirroring group list.
********************************************************************************************/
void addToMG(const Policy&);
/*******************************************************************************************
* @brief Remove this peer to mirroring group
*
* @details
* Set the flag isMirroring to false and remove peer from mirroring group list.
********************************************************************************************/
	void removeFromMG();
/*******************************************************************************************
* @brief Send notification to all the mirroring peers
*
* @param[in] note				The notification
*
* @details
* Send the noteString to all the peers in the mirroring group
********************************************************************************************/
	void sendNoteToMG(const Note&);
};

#endif