#ifndef ENTRY_H
#define ENTRY_H

#include <boost/asio.hpp>
#include "mutable_data.H"
#include "spaddress.h"
#include <queue>
#include "common.hpp"

using namespace boost;
class Entry;
typedef std::pair<Response, Entry*> ResponsePair;
typedef std::pair<Response, short> ResponseTTL;

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
    unsigned short _tmAfterLastChrg() const;

	static std::queue<Entry*> entryRecycler;			//!< Entry's deleted from directory.
	static std::mutex recycleLock;						//!< Lock this mutex before using recycler.

	SPaddress _spAddress;								//!< Source pair address of the entry.
	Permission _permission;								//!< Level of privilage needed by the peer to execute commands.
	unsigned int _timeToLive;							//!< Keep the time to live for this entry.

	bool _iswithPeer = false;							//!< True if this entry is associated with a peer.
	posix_time::ptime _lastChargT;						//!< The time at which this entry was last charged.
	posix_time::ptime _createdT;						//!< The time at which this entry was added to the directory.

	std::string _uid;									//!< Base 64 encoding of the source pair address.	
	std::string _ipAddress;								//!< IPaddress associated with the entry.
	std::string _portNumber;							//!< Port number associated with the entry.
	std::string _description;							//!< Description associated with the entry.

/*******************************************************************************************
* @brief Delegate constructor
*
* @param[in] spAddr				Source Pair address of the entry
*
* @details
* Initialize the values of portNumber, uid, ipAddress, created time and description
********************************************************************************************/
	Entry(const SPaddress&);

public:
/*******************************************************************************************
* @brief recycle the Entry
*
* @param[in] entryPtr			Entry to be recycled
********************************************************************************************/
	static void recycleEntry(Entry*);
/*******************************************************************************************
* @brief Entry constructor
*
* @param[in] spAddr				Source Pair address of the entry
* @param[in] mutData			Mutable data
* @param[in] maxPriv			Maximum privilege cmdSPA have on spAddr
*
* @details
* Initialize the values of description, permission and timeToLive.
* If mutData doesn't have permission - default permission is generated.
********************************************************************************************/
	Entry(const SPaddress&, const MutableData&, const Privilege);
/*******************************************************************************************
* @brief Return true if the entry have expired
*
* @return						True if the entry have expired
*
* @details
* [Not thread safe]
********************************************************************************************/
	bool expired() const;
/*******************************************************************************************
* @brief Get number of minutes after which the entry expires
*
* @return						Number of minutes to expiry
*
* @details
* Use this methord only if the entry is in the directory.
* [Not thread safe]
********************************************************************************************/
	short getTTL() const;
	/*******************************************************************************************
	* @brief Return the UID of the Entry
	*
	* @return						UID of Entry
	* [Not thread safe]
	********************************************************************************************/
	const std::string& uid() const;


/*******************************************************************************************
* @brief Print the Expanded info - IPversion, UID, IPaddress, PortNumber, permission, description
*
* @param[out] strBuffer			String buffer to which the data will be written.
********************************************************************************************/
	void printExpand(std::string&);
/*******************************************************************************************
* @brief Print the Expanded info - IPversion, UID, IPaddress, PortNumber
*
* @param[out] strBuffer			String buffer to which the data will be written.
********************************************************************************************/
	void printBrief(std::string&) const;



/*******************************************************************************************
* @brief Charge the entry (Increment TTL)
*
* @return						The time remaining for that entry.
*
* @details
* Update lastChargeT to the current univerasl time.
* Do telescopic increment to the TTL
* [Not thread safe]
********************************************************************************************/
	short charge();
/*******************************************************************************************
* @brief Return true if the Entry's policy is compatible with policy
*
* @param[in] policy				The Policy to check compatibility with.
* @return						True if have a compatible policy.
********************************************************************************************/
	bool haveSamePolicy(const Policy&);
/*******************************************************************************************
* @brief Check if the commanding spAddr have the proper authority to charge/remove
*
* @param[in] spAddr				The source pair address of the commanding peer.
* @return						True if the peer have the authority to charge/remove.
*
* @details
* Take the maximum privilege between the commanding and targer spAddr and check if have privilege.
* [Not thread safe]
********************************************************************************************/
	bool canChargeWith(const SPaddress&) const;
	bool canRemoveWith(const SPaddress&) const;
/*******************************************************************************************
* @brief Check and update Entry's mutable data with new data
*
* @param[in] spAddr				The source pair address of the commanding peer.
* @param[in] data				Mutable data (with or without permission).
* @return						False if no privilege.
*
* @details
* Take the maximum privilege between the commanding and target spAddr and check if have privilege to change.
* Update the permission if it's valid for the spAddr commanding entry.
* Update the description if available.
* [Not thread safe]
********************************************************************************************/
	bool tryUpdateEntry(const SPaddress&, const MutableData&);
};
#endif