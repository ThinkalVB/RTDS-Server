#ifndef ENTRY_H
#define ENTRY_H

#include <boost/asio.hpp>
#include "mutable_data.H"
#include "spaddress.h"
#include "common.hpp"

using namespace boost;
class Entry;
typedef std::pair<ResponseData, Entry*> AdvResponse;

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
	unsigned int _tmAfterLastChrg() const;
	/*******************************************************************************************
	* @brief Charge the entry (Increment TTL)
	*
	* @details
	* Update lastChargeT to the current univerasl time.
	* Do telescopic increment to the TTL
	* [Not thread safe]
	********************************************************************************************/
	void _charge();
	/*******************************************************************************************
	* @brief Update the time left ( check for expiry )
	*
	* @details
	* [Not thread safe]
	********************************************************************************************/
	void _updateTimeLeft();
	/*******************************************************************************************
	* @brief Initialize base values
	*
	* @param[in] mutData			Mutable data (_policy)
	* @param[in] maxPriv			Maximum privilege cmdSPA have on spAddr
	*
	* @details
	* Initialize the values of _createdT, _lastChargT, _timeToLive, _timeLeft.
	* Initialize default permission and description if not provided.
	* [Not thread Safe]
	********************************************************************************************/
	void _initialize(const MutableData&, const Privilege&);
	/*******************************************************************************************
	* @brief Entry constructor
	*
	* @param[in] spAddr				Source Pair address of the entry
	* @param[in] mutData			Mutable data
	* @param[in] maxPriv			Maximum privilege cmdSPA have on spAddr
	*
	* @details
	* Initialize the values of portNumber, uid, ipAddress
	* If mutData doesn't have permission - default permission is generated.
	********************************************************************************************/
	Entry(const SPaddress&, const MutableData&, const Privilege);

	SPaddress _spAddress;								//!< Source pair address of the entry.
	std::mutex _entryLock;								//!< Lock this mutex before modfying entry. 
	unsigned int _timeToLive;							//!< TTL for this entry.
	unsigned int _timeLeft;								//!< Number of minutes left for the entry.							

	bool _expired;										//!< True if the entry have expired.
	posix_time::ptime _lastChargT;						//!< The time at which this entry was last charged.
	posix_time::ptime _createdT;						//!< The time at which this entry was added to the directory.
	Policy _policy;										//!< Policy with Description and permission

	std::string _uid;									//!< Base 64 encoding of the source pair address.	
	std::string _ipAddress;								//!< IPaddress associated with the entry.
	std::string _portNumber;							//!< Port number associated with the entry.

public:
/*******************************************************************************************
* @brief Create an entry to be added to the directory
*
* @param[in] spAddr				Source Pair address of the entry.
* @param[in] mutData			Mutable data.
* @return						Pointer to the new Entry.
********************************************************************************************/
	static const AdvResponse makeEntry(const SPaddress&, const SPaddress&, const MutableData&);
/*******************************************************************************************
* @brief Get number of minutes after which the entry may expire
*
* @return						Time left and ( SUCESS or NO_EXIST or NO_PRIVILEGE )
********************************************************************************************/
	const ResponseData getTTL();
/*******************************************************************************************
* @brief Get the current policy of the entry
*
* @return						Entry policy.
********************************************************************************************/
	const ResponseData getPolicy();
/*******************************************************************************************
* @brief Print brief info if _policy is compatible with policyMD
*
* @param[out] writeBuffer		Buffer to which brief info will be written.
* @param[in] policyMD			A MutableData with a valid Policy.
* @return						True if the Info is printed.
*
* @details
* Print the Version, IP address and port number of the entry.
********************************************************************************************/
	bool printIfComparable(std::string&, const MutableData&);
/*******************************************************************************************
* @brief Check and charge Entry's timeToLive
*
* @param[in] spAddr				The source pair address of the commanding peer.
* @return						Time left and ( SUCESS or NO_EXIST or NO_PRIVILEGE )
*
* @details
* Take the maximum privilege between the commanding and target spAddr and check if have privilege to change.
* Charge the Entry if it have adequate permission.
********************************************************************************************/
	const ResponseData chargeWith(const SPaddress&);
/*******************************************************************************************
* @brief Shedule removal if the entry can be removed
*
* @param[in] spAddr				The source pair address of the commanding peer.
* @return						SUCESS or NO_EXIST or NO_PRIVILEGE
*
* @details
* Take the maximum privilege between the commanding and target spAddr and check if have privilege to remove.
********************************************************************************************/
	const ResponseData removeWith(const SPaddress&);
/*******************************************************************************************
* @brief Check and update Entry's mutable data with new data
*
* @param[in] spAddr				The source pair address of the commanding peer.
* @param[in] data				Mutable data (with or without permission).
* @return						SUCCESS or NO_EXIST or NO_PRIVILEGE
*
* @details
* Take the maximum privilege between the commanding and target spAddr and check if have privilege to change.
* Update the permission if it's valid for the spAddr commanding entry.
* Update the description if available.
********************************************************************************************/
	const ResponseData updateWith(const SPaddress&, const MutableData&);
/*******************************************************************************************
* @brief Try Re-Initializing this entry as a new entry
*
* @param[in] cmdSPA				The source pair address of the commanding peer.
* @param[in] data				Mutable data (with or without permission).
* @return						SUCCESS or NO_PRIVILEGE
*
* @details
* Take the maximum privilege between the commanding and target spAddr and check if have privilege to change.
* Update the permission if it's valid for the spAddr commanding entry.
* Update the description if available.
********************************************************************************************/
	const ResponseData reAddWith(const SPaddress&, const MutableData&);
};
#endif