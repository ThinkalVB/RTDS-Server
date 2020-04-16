#pragma once
#include "common.hpp"
#include <string>
#include <boost/asio.hpp>

using namespace boost;

/*******************************************************************************************
 * @brief Base class for every entry into the directory.
 *
 * @details
 * Base class for the Entry for IPv4 and IPV6
 ********************************************************************************************/
class entryBase {
protected:
	Permission permission;						//!< Level of privilage needed by the peer to execute commands.
	TTL timeToLive;								//!< Keep the time to live for this entry.

	bool iswithPeer = false;					//!< True if this entry is associated with a peer.
	bool isInDirectory = false;					//!< True if the entry is a directory entry.
	posix_time::ptime addedTime;				//!< The time at which this entry was added to the directory.

/*******************************************************************************************
* @brief Charge the entry extending it's lifetime
*
* @details
* Update the addedTime with the current time if the entry is not online.
********************************************************************************************/
	void _chargeEntry();

public:
/*******************************************************************************************
* @brief Make this entry part of a peer
*
* @details
* Turn the isWithPeer to true which will prevent the directory from deleting this entry.
* Change the TTL to TTL::CONNECTED_TTL - showing it won't expire.
********************************************************************************************/
	void attachToPeer();
/*******************************************************************************************
* @brief Detach the entry from peer
*
* @details
* Turn the isWithPeer to false so that directory can delete this entry after TTL.
* Change the TTL to TTL::RESTRICTED_TTL - flagging it will expire.
********************************************************************************************/
	void detachFromPeer();
/*******************************************************************************************
* @brief Check if the entry have expired
*
* @param[out] expireIn			Return hom many minutes after which this entry will expire.
* @return						True if the entry have expired.
*
* @details
* Take the time difference between timeNow and addedTime and check if expired.
********************************************************************************************/
	bool haveExpired(short&);
/*******************************************************************************************
* @brief Check if the entry have expired
*
* @return						True if the entry have expired.
*
* @details
* Take the time difference between timeNow and addedTime and check if expired.
********************************************************************************************/
	bool haveExpired();

	std::mutex accessLock;						//!< Lock these mutex when accessing data
	std::string UID;							//!< Base 64 encoding of the source pair address.	
	std::string ipAddress;						//!< IPaddress associated with the entry.
	std::string portNumber;						//!< Port number associated with the entry.
	std::string description;					//!< Description associated with the entry.
};


/*******************************************************************************************
 * @brief EntryV4 contains all data needed for an Entry of type IPV4
 ********************************************************************************************/
class EntryV4 : public entryBase
{
/*******************************************************************************************
* @brief Make IPV4 Entry object
********************************************************************************************/
	EntryV4() {}
	sourcePairV4 sourcePair;					//!< Source Pair address (IPaddress + portNumber) Network Byte order.
public:
	static const std::string versionID;			//!< The version ID of the entry "v4" for IPV4
	friend class Directory;
};


/*******************************************************************************************
 * @brief EntryV6 contains all data needed for an Entry of type IPV6
 ********************************************************************************************/
class EntryV6 : public entryBase
{
/*******************************************************************************************
* @brief Make IPV6 Entry object
********************************************************************************************/
	EntryV6() {}
	sourcePairV6 sourcePair;					//!< Source Pair address (IPaddress + portNumber) Network Byte order.
public:
	static const std::string versionID;			//!< The version ID of the entry "v6" for IPV6
	friend class Directory;
};


/*******************************************************************************************
 * @brief Union to hold the poiter to either EntryV4 or EntryV6
 ********************************************************************************************/
union Entry
{
	EntryV4* Ev4;
	EntryV6* Ev6;
};