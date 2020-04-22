#pragma once
#include "common.hpp"
#include "cppcodec/base64_rfc4648.hpp"
#include <variant>
#include <boost/asio.hpp>

using namespace boost;

/*******************************************************************************************
 * @brief Base class for every entry into the directory.
 *
 * @details
 * Base class for the Entry for IPv4 and IPV6
 * The member functions of this class in not thread safe. Directory class must guarantee safety.
 ********************************************************************************************/
class __base_entry
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

protected:
	static const std::string VER[];				//!< All version code in string.
	static const char PRI[];				    //!< All privilage code in string.

	Version version;							//!< Version of the derived class IPV4 or IPV6.
	Permission permission;						//!< Level of privilage needed by the peer to execute commands.
	TTL timeToLive;								//!< Keep the time to live for this entry.

	bool iswithPeer = false;					//!< True if this entry is associated with a peer.
	bool isInDirectory = false;					//!< True if the entry is a directory entry.
	posix_time::ptime lastChargT;				//!< The time at which this entry was last charged.
	posix_time::ptime createdT;					//!< The time at which this entry was added to the directory.

	std::mutex accessLock;						//!< Lock these mutex when accessing or modfying data.
	std::string UID;							//!< Base 64 encoding of the source pair address.	
	std::string ipAddress;						//!< IPaddress associated with the entry.
	std::string portNumber;						//!< Port number associated with the entry.
	std::string description;					//!< Description associated with the entry.
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

public:
/*******************************************************************************************
* @brief Print the UID
*
* @param[out] strBuffer			String buffer to which the data will be written.
********************************************************************************************/
	void printUID(std::string&);
/*******************************************************************************************
* @brief Print the brief info - IPversion, IPaddress, PortNumber
*
* @param[out] strBuffer			String buffer to which the data will be written.
********************************************************************************************/
	void printBrief(std::string&);
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
	Version getVersion();
	friend class Directory;
};


template <typename SP, typename IPaddrT>
class SPentry : public __base_entry
{
	SPentry(SP&, IPaddrT&, unsigned short);
	const SP sourcePair;

/*******************************************************************************************
* @brief Return the maximum privilege the command issuing entry have
*
* @param[in] cmdEntry			The ptr to the command issuing Entry.
* @return						The maximum privilage the comEntry have on entry.
*
* @details
* If both IP address are not the same then - return Privilege::LIBERAL_ENTRY;
* Both have the same IP  version address - return Privilege::PROTECTED_ENTRY;
********************************************************************************************/
	Privilege maxPrivilege(__base_entry*);
	friend class Directory;
};

/*******************************************************************************************
* @brief Construct SPentry of type EntryV4 or EntryV6
*
* @param[in] srcPair			SourcePair type V4 or V6
* @param[in] ipAdd				IP address IPv4 or IPv6
* @param[in] portNum			Port Number of the sourcePair
*
* @details
* Create a sourcePair of type V4 or V6 ( only use with directory ). Initialize values.
********************************************************************************************/
template <typename SP, typename IPaddrT>
SPentry<SP, IPaddrT>::SPentry(SP& srcPair, IPaddrT& ipAdd, unsigned short portNum) : sourcePair(srcPair)
{
	portNumber = std::to_string(portNum);
	ipAddress = ipAdd.to_string();
	UID = cppcodec::base64_rfc4648::encode(sourcePair);
	createdT = posix_time::second_clock::universal_time();
	description = "[]";

	if constexpr (std::is_same_v<IPaddrT, asio::ip::address_v4>)
		version = Version::V4;
	else
		version = Version::V6;
}

typedef SPentry<sourcePairV4, asio::ip::address_v4> EntryV4;
typedef SPentry<sourcePairV6, asio::ip::address_v6> EntryV6;


/*******************************************************************************************
 * @brief Union to hold the poiter to either EntryV4 or EntryV6
 ********************************************************************************************/
union Entry
{
	__base_entry* Ev;
	EntryV4* Ev4;
	EntryV6* Ev6;
};

/*******************************************************************************************
 * @brief Struct to hold both sourcePairV4 and sourcePairV6
 ********************************************************************************************/
struct SourcePair
{
	union __source_pair
	{
		sourcePairV4 V4;
		sourcePairV6 v6;
	}SP;
	Version version;
};

template<typename SP, typename IPaddrT>
inline Privilege SPentry<SP, IPaddrT>::maxPrivilege(__base_entry* cmdEntry)
{
	if (version == cmdEntry.version)
	{
		if (std::memcmp((void*)sourcePair[0], (void*)((SPentry*)cmdEntry)->sourcePair[0], (sourcePair.size() - 2)) == 0)
			return Privilege::PROTECTED_ENTRY;
		else
			return Privilege::LIBERAL_ENTRY;
	}
	else
		return Privilege::LIBERAL_ENTRY;
}
