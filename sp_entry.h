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
	SourcePair sourcePair;						//!< Source Pair address (IPaddress + portNumber) Network Byte order.
	Permission permission;						//!< Level of privilage needed by the peer to execute commands.

	bool iswithPeer = false;					//!< True if this entry is associated with a peer.
	bool isInDirectory = false;					//!< True if the entry is a directory entry.
	unsigned int ttl;							//!< The Time To Live for the entry.
	posix_time::ptime startTime;				//!< The time at which this entry was added to the directory.

public:
/*******************************************************************************************
* @brief Make this entry part of a peer
*
* @details
* Turn the isWithPeer to true which will prevent the directory from deleting this entry.
********************************************************************************************/
	void attachToPeer();
/*******************************************************************************************
* @brief Detach the entry from peer
*
* @details
* Turn the isWithPeer to false so that directory can delete this entry after TTL.
********************************************************************************************/
	void detachFromPeer();
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