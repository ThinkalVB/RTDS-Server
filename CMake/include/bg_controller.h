#ifndef BG_CONTROLLER_H
#define BG_CONTROLLER_H

#include <shared_mutex>
#include <vector>
#include <map>

#include "tcp_peer.h"
#include "udp_peer.h"
#include "message.h"

class BGroupUnrestricted
{
protected:
	std::string mBgID;								// Broadcast Group ID

	std::shared_mutex mTCPpeerListLock;				// Peer list lock
	std::vector<TCPpeer*> mTCPpeerList;				// Peers in the Broadcast group

public:
/*******************************************************************************************
* @brief Add a peer to the peer list
*
* @param[in]			Pointer to the peer
********************************************************************************************/
	void addPeer(TCPpeer*);
/*******************************************************************************************
* @brief Remove a peer from the peer list
*
* @param[in]			Pointer to the peer
********************************************************************************************/
	void removePeer(TCPpeer*);
/*******************************************************************************************
* @brief Constructor
*
* @param[in]			Broadcast group ID
********************************************************************************************/
	BGroupUnrestricted(const std::string&);
/*******************************************************************************************
* @brief Check if the peer list is empty
*
* @return				True if the peer list is empty
********************************************************************************************/
	bool isEmpty() const;
/*******************************************************************************************
* @brief Broadcast a message to all peers in the peer list
*
* @param[in]			Message
* @param[in]			Peer's Broadcast Group Tag
********************************************************************************************/
	void broadcast(const Message*, const std::string_view&);
	void broadcast(const Message*);
};

class BGroup : BGroupUnrestricted
{
public:
/*******************************************************************************************
* @brief Broadcast a message to all peers in the peer list (except the calling peer)
*
* @param[in]			Message broadcasting peer
* @param[in]			Message
* @param[in]			Broadcast Group Tag (for tag specific broadcast)
********************************************************************************************/
	void broadcast(TCPpeer*, const Message*, const std::string_view&);
	void broadcast(TCPpeer*, const Message*);
};

class BGcontroller
{
	static std::map<std::string, BGroupUnrestricted*> mBGmap;		// Broadcast Group Map				
	static std::shared_mutex mBgLock;								// Mutex for thread safety
public:
/*******************************************************************************************
* @brief Add the peer to the broadcast group
*
* @param[in]			Peer
* @param[in]			Broadcast Group ID
* @return				Pointer to the Broadcast group in which peer is added
*
* @details
* Return null pointer if the operation fails
********************************************************************************************/
	static BGroup* addToBG(TCPpeer*, const BGID&);
/*******************************************************************************************
* @brief Remove a peer from the broadcast group
*
* @param[in]			Peer
* @param[in]			Broadcast Group ID
********************************************************************************************/
	static void removeFromBG(TCPpeer*, const BGID&);
/*******************************************************************************************
* @brief Broadcast a message to all peers in the peer list
*
* @param[in]			Message
* @param[in]			Broadcast Group ID
* @param[in]			Broadcast Group Tag
********************************************************************************************/
	static void broadcast(const Message*, const BGID);
	static void broadcast(const Message*, const BGID, const std::string_view&);
};

#endif
