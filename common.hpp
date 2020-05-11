#ifndef COMMON_H
#define COMMON_H


#define DEBUG
/* #define RTDS_DUAL_STACK */

#define MAX_NOTE_SIZE 100

#define V4_UID_MAX_CHAR 8
#define V6_UID_MAX_CHAR 24

#define PORT_NUM_MAX_CHAR 5
#define MAX_PORT_NUM_VALUE 65535
#define MAX_DESC_SIZE 202
#define RTDS_BUFF_SIZE 300

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
********************************************************************************************/
enum class Privilege : char
{
	LIBERAL_ENTRY		= 0,
	PROTECTED_ENTRY		= 1,
	RESTRICTED_ENTRY	= 2
};

/*******************************************************************************************
* @brief Enum class represeting maximum TTL of different entrys
*
* @details
* CONNECTED_TTL		This entry won't expire hence value -1.
* LIBERAL_TTL		This entry can survice upto 10 minutes.
* PROTECTED_TTL		This entry can survive upto 30 minutes.
* RESTRICTED_TTL	This entry can survive upto 60 minutes.
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
	FLUSH	= 5,
	UPDATE	= 6,
	REMOVE	= 7,
	COUNT	= 8,
	MIRROR	= 9,
	LEAVE	= 10,
	EXIT	= 11
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

/*******************************************************************************************
 * @brief Struct to hold both SourcePairV4 and SourcePairV6
 ********************************************************************************************/
struct SPAddress
{
	union __sp_address
	{
		SourcePairV4 V4;
		IPVersion4 IPV4;
		SourcePairV6 V6;
		IPVersion6 IPV6;
	}SPA;
	Version version;
};
#endif