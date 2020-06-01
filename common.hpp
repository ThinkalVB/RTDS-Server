#ifndef COMMON_H
#define COMMON_H


#define DEBUG
/* #define RTDS_DUAL_STACK */

#define MAX_TTL 24*60					//!< Maximum telescopic TTL	(24hrs * 60min)
#define RTDS_PORT 389					//!< Default RTDS port number

#define STR_V4 "v4"						//!< Version V4 in string
#define STR_V6 "v6"						//!< Version V6 in string

#define MAX_NOTE_SIZE 200				//!< Maximum number of notifications to be cached

#define V4_UID_MAX_CHAR 8				//!< 8 characters for V4 UID
#define V6_UID_MAX_CHAR 24				//!< 24 characters for V6 UID

#define PORT_NUM_MAX_CHAR 5				//!< Maximum number of digits as port number
#define MAX_PORT_NUM_VALUE 65535		//!< Maximum value for port number
#define MAX_DESC_SIZE 202				//!< Maximum size of description including '[' and ']'
#define RTDS_BUFF_SIZE 300				//!< Maximum size of the readBuffer

#include <cstdint>
#include <array>

typedef std::array<uint8_t, 6> SourcePairV4;
typedef std::array< unsigned char, 4 > IPVersion4;
typedef std::array<uint8_t, 18> SourcePairV6;
typedef std::array< unsigned char, 16 > IPVersion6;
typedef std::array<char, RTDS_BUFF_SIZE> ReceiveBuffer;

/*******************************************************************************************
* @brief Enum class representing the levels of privilege
*
* @details
* LIBERAL_ENTRY		All Peers with any IP address and port number will have authority.
* PROTECTED_ENTRY	All Peers with the same IP address will have authority.
* RESTRICTED_ENTRY	Only peer with the same IP address and port number have authority.
* ALL_ENTRY			Either 3 of the above.
********************************************************************************************/
enum class Privilege : char
{
	LIBERAL_ENTRY		= 0,
	PROTECTED_ENTRY		= 1,
	RESTRICTED_ENTRY	= 2,
	ALL_ENTRY			= 3
};

/*******************************************************************************************
* @brief Enum class representing the starting TTL of entrys with different maxPrivilege
*
* @details
* LIBERAL_TTL		10 minutes.
* PROTECTED_TTL		30 minutes.
* RESTRICTED_TTL	60 minutes.
********************************************************************************************/
enum class TTL
{
	LIBERAL_TTL		= 10,
	PROTECTED_TTL	= 30,
	RESTRICTED_TTL	= 60
};

/*******************************************************************************************
* @brief Enum class for Response 
*
* @details
* ok_success		Command is successful.
* bad_command		Bad command.
* choose_dir		Switch to a directory before using the command.
* bad_param			Bad parameter provided with the command.
* no_exist			No compatible resources exist.
* redudant_data		Entry or directory already exists.
* wait_retry		Wait and try again after sometime.
* no_privilege		Command doesn’t have the apt privilege.
********************************************************************************************/
enum class Response
{
	REDUDANT_DATA	= 0,
	SUCCESS			= 1,
	NO_PRIVILAGE	= 2,
	BAD_COMMAND		= 3,
	BAD_PARAM		= 4,
	NO_EXIST		= 5,
	WAIT_RETRY		= 6
};

/*******************************************************************************************
* @brief Enum class for Version
*
* @details
* V4				Version 4 (IPV4)
* V6				Version 6 (IPV6)
********************************************************************************************/
enum class Version
{
	V4 = 0,
	V6 = 1
};

/*******************************************************************************************
* @brief Enum class for Command
*
* @details
* ping				Ping with the RTDS server.
* register			Register a new directory.
* switch			Switch to the scope of a directory.
* add				Add a new entry to the directory.
* search			Search for an entry in the directory.
* ttl				Find the Time To Live for an entry.
* charge			Increase the TTL for an entry.
* update			Update the values of an entry.
* remove			Remove an entry from the directory.
* count				Count the number of entries in the directory.
* mirror			Mirror the progress in a directory.
* leave				Exit the mirroring group.
* exit				Exit from the scope of the directory.
* delete			Delete the directory.
********************************************************************************************/
enum class Command
{
	PING	= 0,
	ADD		= 1,
	SEARCH	= 2,
	TTL		= 3,
	CHARGE	= 4,
	UPDATE	= 5,
	REMOVE	= 6,
	MIRROR	= 7,
	LEAVE	= 8,
	EXIT	= 9
};

/*******************************************************************************************
* @brief Individual privilage for different operation
*
* @details
* This structure store the minimum individual privilage levels needed by the command issuing authority
* to do the remove, change or charge operations.
* remove		Remove the entry from directory.
* change		Change the values in permission or description.
* charge		Extend the life time of an entry.
********************************************************************************************/
struct Permission
{
	Privilege charge;
	Privilege change;
	Privilege remove;
};
#endif