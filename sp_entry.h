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
	entryBase() {}
	~entryBase() {}

	SourcePair sourcePair;						//!< Source Pair address (IPaddress + portNumber) Network Byte order.
	Permission permission;						//!< Level of privilage needed by the peer to execute commands.

	std::string UID;							//!< Base 64 encoding of the source pair address				
	std::string ipAddress;						//!< IPaddress associated with the entry.
	std::string portNumber;						//!< Port number associated with the entry.
	std::string description;					//!< Description associated with the entry.

	unsigned int ttl;							//!< The Time To Live for the entry.
	posix_time::ptime startTime;				//!< The time at which this entry was added to the directory.
};

/*******************************************************************************************
 * @brief EntryV4 contains all data needed for an Entry of type IPV4
 ********************************************************************************************/
class EntryV4 : private entryBase
{
	static const std::string versionID;			//!< The version ID of the entry "v4" for IPV4
public:
/*******************************************************************************************
* @brief Initilaize the values of the entry 
*
* @param[in] ipAdd				IPv4 address associated with the entry
* @param[in] portNum			Port number associated with the entry
*
* @details
* Generate base64 UID, binary sourcePair address(host byte order)
********************************************************************************************/
	EntryV4(asio::ip::address_v4, unsigned short);
	friend class CmdInterpreter;
};

/*******************************************************************************************
 * @brief EntryV6 contains all data needed for an Entry of type IPV6
 ********************************************************************************************/
class EntryV6 :private entryBase
{
	static const std::string versionID;			//!< The version ID of the entry "v6" for IPV6
public:
/*******************************************************************************************
* @brief Initilaize the values of the entry
*
* @param[in] ipAdd				IP64 address associated with the entry
* @param[in] portNum			Port number associated with the entry
*
* @details
* Generate base64 UID, binary sourcePair address(host byte order)
********************************************************************************************/
	EntryV6(asio::ip::address_v6, unsigned short);
	friend class CmdInterpreter;
};

/*******************************************************************************************
 * @brief Union to hold the poiter to either EntryV4 or EntryV6
 ********************************************************************************************/
union Entry
{
	EntryV4* Ev4;
	EntryV6* Ev6;
};