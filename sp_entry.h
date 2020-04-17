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
	static const std::string VER[];				//!< All version code in string.
	static const char PRI[];				    //!< All version code in string.	

	Version version;							//!< Version of the derived class IPV4 or IPV6.
	Permission permission;						//!< Level of privilage needed by the peer to execute commands.
	TTL timeToLive;								//!< Keep the time to live for this entry.

	bool iswithPeer = false;					//!< True if this entry is associated with a peer.
	bool isInDirectory = false;					//!< True if the entry is a directory entry.
	posix_time::ptime lastChargT;				//!< The time at which this entry was last charged.

	std::mutex accessLock;						//!< Lock these mutex when accessing or modfying data.
	std::string UID;							//!< Base 64 encoding of the source pair address.	
	std::string ipAddress;						//!< IPaddress associated with the entry.
	std::string portNumber;						//!< Port number associated with the entry.
	std::string description;					//!< Description associated with the entry.

/*******************************************************************************************
* @brief Charge the entry extending it's lifetime
*
* @details
* Update the addedTime with the current time if the entry is not online.
********************************************************************************************/
	void _chargeEntry();
/*******************************************************************************************
* @brief Check if the entry have expired
*
* @return						True if the entry have expired.
*
* @details
* Take the time difference between timeNow and addedTime and check if expired.
********************************************************************************************/
	bool _haveExpired();
/*******************************************************************************************
* @brief Return the time passed after the last charge [Not thread safe]
*
* @return						Time in minutes, passed after last charge.
********************************************************************************************/
	short _getTimePassed();
public:
/*******************************************************************************************
* @brief Return true if the entry is in directory
*
* @return						True if entry is in directory.
********************************************************************************************/
	bool inDirectory();
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
* @brief Print the UID to the string buffer
*
* @param[out] strBuffer			String buffer to which the data will be written.
*
* @details
* Do not use print functions in combinations. All print functions must be Individualistic.
********************************************************************************************/
	void printUID(std::string&);
/*******************************************************************************************
* @brief Print the Expanded info - IPversion, UID, IPaddress, PortNumber, permission, description
*
* @param[out] strBuffer			String buffer to which the data will be written.
*
* @details
* Do not use print functions in combinations. All print functions must be Individualistic.
********************************************************************************************/
	void printExpand(std::string&);
/*******************************************************************************************
* @brief Print the UID to the string buffer.
*
* @param[out] strBuffer			String buffer to which the data will be written.
*
* @details
* Do not use print functions in combinations. All print functions must be Individualistic.
********************************************************************************************/
	void printEntryCount(std::string&);
/*******************************************************************************************
* @brief Print the brief info - IPversion, IPaddress, PortNumber
*
* @param[out] strBuffer			String buffer to which the data will be written.
*
* @details
* Do not use print functions in combinations. All print functions must be Individualistic.
********************************************************************************************/
	void printBrief(std::string&);
/*******************************************************************************************
* @brief Print the time To Live for the entry
*
* @param[out] strBuffer			String buffer to which the data will be written.
*
* @details
* Do not use print functions in combinations. All print functions must be Individualistic.
* Find the difference between the current time and the lastChargeT time. Check if it's beyond the TTL.
********************************************************************************************/
	void printTTL(std::string&);
	friend class Directory;
};


/*******************************************************************************************
 * @brief EntryV4 contains all data needed for an Entry of type IPV4
 ********************************************************************************************/
class EntryV4 : public entryBase
{
/*******************************************************************************************
* @brief Make IPV4 Entry object
********************************************************************************************/
	EntryV4();
	sourcePairV4 sourcePair;					//!< Source Pair address (IPaddress + portNumber) Network Byte order.
public:
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
	EntryV6();
	sourcePairV6 sourcePair;					//!< Source Pair address (IPaddress + portNumber) Network Byte order.
public:
	friend class Directory;
};


/*******************************************************************************************
 * @brief Union to hold the poiter to either EntryV4 or EntryV6
 ********************************************************************************************/
union Entry
{
	entryBase* Ev;
	EntryV4* Ev4;
	EntryV6* Ev6;
};