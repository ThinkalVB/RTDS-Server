#ifndef STREAM_PEER_H
#define STREAM_PEER_H

#include "peer.h"
#include <asio/ip/tcp.hpp>
#include <asio/ssl.hpp>
#include <atomic>
#include <shared_mutex>
#include "message.h"

typedef asio::ssl::stream<asio::ip::tcp::socket> SSLsocket;

class BGroup;
class StreamPeer : public Peer
{		
	static std::atomic_int mGlobalPeerCount;		// Keep the total count of peers

protected:
	PeerMode mPeerMode;								// Hearing mode of the peer
	BGroup* mBgPtr;									// Pointer to broadcast group
	BGID mBgID;										// Broadcast group ID
	BGT mBgTag;										// Broadcast group Tag
	SAP mSApair;									// SAP string of the peer
	PeerType mPeerType;								// Peer Type of the peer

	std::shared_mutex mPeerResourceMtx;				// Mutex for locking the shared resources
	bool mPeerIsActive;								// True if the peer socket is operational
	bool mIsInBG;									// True if this peer is in Broadcast Group

/*******************************************************************************************
* @brief Constructor [Increment global peer count]
********************************************************************************************/
	StreamPeer();
/*******************************************************************************************
* @brief Distructor [Decrement the global peer count]
********************************************************************************************/
	~StreamPeer();

public:
	virtual void sendMessage(const Message*) = 0;
/*******************************************************************************************
* @brief Get the peer type
*
* @return			Peer type
*
* @details
* Send NOT_IN_BG if peer is not in a broadcast group
* Send SUCCESS if the changing was success
********************************************************************************************/
	const PeerType peerType() const;
/*******************************************************************************************
* @brief Get the total Global Peer count
********************************************************************************************/
	int getPeerCount() const;

/*******************************************************************************************
* @brief Disconnect the peer and delete the object
********************************************************************************************/
	void disconnect();
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
* @brief Leave the brodcast group
********************************************************************************************/
	void leaveBG();
/*******************************************************************************************
* @brief Broadcast a message to the group
*
* @param[in]			Message
* @param[in]			Broadcast Group Tag
********************************************************************************************/
	void broadcastTo(const std::string_view&, const std::string_view&);
/*******************************************************************************************
* @brief Message to a group (with SAP string)
*
* @param[in]			Message
* @param[in]			Broadcast Group Tag
********************************************************************************************/
	void messageTo(const std::string_view&, const std::string_view&);
};

#endif