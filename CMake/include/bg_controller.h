#ifndef BG_CONTROLLER_H
#define BG_CONTROLLER_H

#include <vector>
#include <map>
#include "peer.h"
#include "message.h"

class BGroupUnrestricted
{
protected:
	std::string mBgID;									// Broadcast Group ID
	std::mutex mPeerListLock;							// Peer list lock
	std::vector<Peer*> mPeerList;						// Peers in the Broadcast group

public:
/*******************************************************************************************
* @brief Add a peer to the peer list
*
* @param[in]			Pointer to the peer
********************************************************************************************/
	void addPeer(Peer*);
/*******************************************************************************************
* @brief Remove a peer from the peer list
*
* @param[in]			Pointer to the peer
********************************************************************************************/
	void removePeer(Peer*);
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
};

class BGroup : BGroupUnrestricted
{
public:
/*******************************************************************************************
* @brief Broadcast a message to all peers in the peer list
*
* @param[in]			Message broadcasting peer
* @param[in]			Message
* @param[in]			Broadcast Group Tag
*
* @details
* The message won't be send to the messaging peer
********************************************************************************************/
	void broadcast(Peer*, const Message*, const std::string_view&);
	void broadcast(Peer*, const Message*);
};

class BGcontroller
{
	static std::map<std::string, BGroupUnrestricted*> mBGmap;		// Broadcast Group Map
	static std::mutex mBgLock;										// Lock this Mutex before insertion
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
	static BGroup* addToBG(Peer*, const BGID&);
/*******************************************************************************************
* @brief Remove a peer from the broadcast group
*
* @param[in]			Peer
* @param[in]			Broadcast Group ID
********************************************************************************************/
	static void removeFromBG(Peer*, const BGID&);
};

#endif
