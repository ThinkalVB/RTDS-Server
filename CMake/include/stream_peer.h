#ifndef STREAM_PEER_H
#define STREAM_PEER_H

#include "peer.h"
#include <asio/error_code.hpp>
#include <atomic>
#include <shared_mutex>

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

	std::shared_mutex mPeerResourceMtx;				// Mutex for locking the shared resources
	bool mPeerIsActive;								// True if the peer socket is operational
	bool mIsInBG;									// True if this peer is in Broadcast Group

	StreamPeer();
	~StreamPeer();

public:
/*******************************************************************************************
* @brief Get the total Global Peer count
********************************************************************************************/
	int getPeerCount();
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
};

#endif