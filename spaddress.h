#ifndef SPADDRESS_H
#define SPADDRESS_H
#include "common.hpp"
#include <boost/asio.hpp>
using namespace boost;

/*******************************************************************************************
 * @brief Class to hold both SourcePairV4 and SourcePairV6
 ********************************************************************************************/
class SPaddress
{
	union __sp_address
	{
		SourcePairV4 V4;					//!< Source pair address version 4.
		IPVersion4 IPV4;					//!< IP address version 4.
		SourcePairV6 V6;					//!< Source pair address version 6.
		IPVersion6 IPV6;					//!< IP address version 6.
	}_SPA;
	Version _version;						//!< Source pair address version.
	unsigned short _portNumber;				//!< Associated port number.

public:
/*******************************************************************************************
* @brief Constructor
*
* @param[in] ipAddr				IPaddress version(4/6).
* @param[in] portNum			Port Number.
********************************************************************************************/
	SPaddress(const asio::ip::address_v4&, unsigned short);
	SPaddress(const asio::ip::address_v6&, unsigned short);
/*******************************************************************************************
* @brief Constructor
*
* @param[in] uid				Base64 encoded source pair UID string.
********************************************************************************************/
	SPaddress(const std::string_view&);
/*******************************************************************************************
* @brief Constructor
*
* @param[in] remoteEp			Asio endpoint of the client system.
********************************************************************************************/
	SPaddress(const asio::ip::tcp::endpoint&);

/*******************************************************************************************
* @brief Return Base64 encoded string for the sourcePair address
*
* @return						Base64 encoded UID.
********************************************************************************************/
	std::string toUID() const;
/*******************************************************************************************
* @brief Return IPaddress as string
*
* @return						IP address as string.
********************************************************************************************/
	std::string ipAddress() const;
/*******************************************************************************************
* @brief Return Version, IPaddress and portNumber as string
*
* @return						Version, IP address, port number as string.
********************************************************************************************/
	std::string briefInfo() const;
/*******************************************************************************************
* @brief Return portNumber as string
*
* @return						Port number as string.
********************************************************************************************/
	std::string portNumber() const;
/*******************************************************************************************
* @brief Return Version of the sourcePair address
*
* @return						Version as string.
********************************************************************************************/
	Version version() const;
/*******************************************************************************************
* @brief Return const reference to Source Pair address V4
*
* @return						SPaddress V4
********************************************************************************************/
	const SourcePairV4& spAddressV4() const;
/*******************************************************************************************
* @brief Return const reference to Source Pair address V6
*
* @return						SPaddress V6
********************************************************************************************/
	const SourcePairV6& spAddressV6() const;
/*******************************************************************************************
* @brief Return maximum privilege between the spAddress and this address
*
* @param[in] spAddr				Source Pair address of the commanding entry.
* @return						The maximum privilege.
********************************************************************************************/
	Privilege maxPrivilege(const SPaddress&) const;
};

#endif