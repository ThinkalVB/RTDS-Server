#ifndef ENTRY_H
#define ENTRY_H

#include "advanced_dll.h"
#include <boost/asio.hpp>
#include <mutex>
#include "common.hpp"

using namespace boost;

/*******************************************************************************************
 * @brief Class for every entry into the directory.
 *
 * @details
 * Base class for the Entry for IPv4 and IPV6
 * The member functions of this class in not thread safe. Directory class must guarantee safety.
 ********************************************************************************************/
class Entry
{
/*******************************************************************************************
* @brief Return the time passed after the last charge
*
* @return						Time in minutes, passed after last charge.
********************************************************************************************/
    short _tmAfterLastChrg();
/*******************************************************************************************
* @brief Turn the isInDirectory flag to false if the entry's TTL have expired
********************************************************************************************/
    void _doExpiryCheck();

	static const std::string VER[];						//!< All version code in string.

	static DLLController<Entry> dllController;			//!< Controller to add and remove entry from directory
	DLLNode<Entry> dllNode;								//!< DLL node to keep track of previous and next entries

	SPAddress spAddress;								//!< Source pair address of the entry.
	Permission permission;								//!< Level of privilage needed by the peer to execute commands.
	TTL timeToLive;										//!< Keep the time to live for this entry.

	bool iswithPeer = false;							//!< True if this entry is associated with a peer.
	bool isInDirectory = false;							//!< True if the entry is a directory entry.
	posix_time::ptime lastChargT;						//!< The time at which this entry was last charged.
	posix_time::ptime createdT;							//!< The time at which this entry was added to the directory.

	std::mutex accessLock;								//!< Lock these mutex when accessing or modfying data.
	std::string UID;									//!< Base 64 encoding of the source pair address.	
	std::string ipAddress;								//!< IPaddress associated with the entry.
	std::string portNumber;								//!< Port number associated with the entry.
	std::string description;							//!< Description associated with the entry.

	Entry(const SourcePairV4&, asio::ip::address_v4, unsigned short);
	Entry(const SourcePairV6&, asio::ip::address_v6, unsigned short);
/*******************************************************************************************
* @brief Get number of minutes after which the entry expires
*
* @return						Number of minutes to expiry
*
* @details
* Use this methord only if the entry is in the directory.
* [Not thread safe]
********************************************************************************************/
	short getTTL();
/*******************************************************************************************
* @brief Print the Expanded info - IPversion, UID, IPaddress, PortNumber, permission, description
*
* @param[out] strBuffer			String buffer to which the data will be written.
* [Not thread safe]
********************************************************************************************/
	void printExpand(std::string&);
/*******************************************************************************************
* @brief Return true if the entry is in directory
*
* @return						True if entry is in directory.
*
* @details
* Do expiry check before returning isInDirectory flag.
* [Not thread safe]
********************************************************************************************/
	bool inDirectory();
/*******************************************************************************************
* @brief Add the entry to the directory
*
* @param[in] ttl				Time to Live for the entry.
*
* @details
* Increment the entryCount, turn isInDirectory flag to true, update lastChargeT.
* [Not thread safe]
********************************************************************************************/
	void addToDirectory(TTL);
/*******************************************************************************************
* @brief Print the Expanded info - IPversion, UID, IPaddress, PortNumber, permission, description
*
* @details
* Decrement the entryCount, turn isInDirectory flag to false.
* [Not thread safe]
********************************************************************************************/
	void removeFromDirectory();
/*******************************************************************************************
* @brief Assign default permission to the entry
*
* @param[in] maxPrivilege		The maximum privilege commanding entry have on this entry
*
* @details
* PROTECTED_ENTRY if either protected or restricted, else LIBERAL_ENTRY.
* [Not thread safe]
********************************************************************************************/
	void assignDefualtPermission(const Privilege&);
/*******************************************************************************************
* @brief Print the brief info - IPversion, IPaddress, PortNumber
*
* @param[out] strBuffer			String buffer to which the data will be written.
********************************************************************************************/
	void printBrief(std::string&);
/*******************************************************************************************
* @brief Charge the entry
*
* @param[in] newTTL				New Time to Live for the entry.
* @return						The time remaining for that entry
*
* @details
* Update lastChargeT to the current univerasl time.
* [Not thread safe]
********************************************************************************************/
	short charge(const TTL&);
/*******************************************************************************************
* @brief Lock this entry of further access from other threads
********************************************************************************************/
	void lock();
/*******************************************************************************************
* @brief Unlock this entry
********************************************************************************************/
	void unlock();
/*******************************************************************************************
* @brief Return the maximum privilage cmdEntry have on this entry
*
* @param[in] cmdEntry			The command issuing entry
* @return						Return the maximum privilage
*
* @details
* [Not thread safe]
********************************************************************************************/
	Privilege maxPrivilege(Entry*);
/*******************************************************************************************
* @brief Return true if have the privilege to charge, change or remove
*
* @param[in] maxPrivilege		The maximum privilege cmdEntry have on entry
* @return						True if have privilege
*
* @details
* These functions are not thread safe. [Need external locking for thread safety]
********************************************************************************************/
	bool canChargeWith(const Privilege&);
	bool canChangeWith(const Privilege&);
	bool canRemoveWith(const Privilege&);

public:
/*******************************************************************************************
* @brief Return the UID of the entry
********************************************************************************************/
	const std::string& uid();
/*******************************************************************************************
* @brief Make this entry part of a peer
*
* @details
* Turn the isWithPeer to true which will prevent the directory from deleting this entry.
* When the entry is with peer it doesn't need charging. TTL is always RESTRICTED_TTL.
********************************************************************************************/
	void attachToPeer();
/*******************************************************************************************
* @brief Detach the entry from peer
*
* @details
* Turn the isWithPeer to false so that directory can delete this entry after TTL.
* Update the lastChargeT for accurate TTL calculation.
********************************************************************************************/
	void detachFromPeer();
/*******************************************************************************************
* @brief Get the version of the entry
*
* @return						Version type V4 or V6
********************************************************************************************/
	const Version& version();

	template<typename T2> friend class DLLNode;
	friend class Tocken;
	friend class Directory;
};
#endif


