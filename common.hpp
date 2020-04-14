#pragma once
#include <cstdint>
#include <array>
#include <string>

typedef std::array<uint8_t, 6> sourcePairV4;
typedef std::array<uint8_t, 18> sourcePairV6;

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
	LIBERAL_ENTRY		= 'l',
	PROTECTED_ENTRY		= 'p',
	RESTRICTED_ENTRY	= 'r'
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
enum class TTL : short
{
	CONNECTED_TTL	= -1,
	LIBERAL_TTL		= 10,
	PROTECTED_TTL	= 30,
	RESTRICTED_TTL	= 60
};

/*******************************************************************************************
* @brief Response 
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
namespace Response{
	const std::string REDUDANT_DATA =	"redudant_data";
	const std::string SUCCESS		=	"ok_success";
	const std::string NO_PRIVILAGE	=	"no_privilege";
	const std::string BAD_COMMAND	=	"bad_command";
	const std::string BAD_PARAM		=	"bad_param";
	const std::string NO_EXIST		=	"no_exist";
	const std::string WAIT_RETRY	=	"wait_retry";
}

/*******************************************************************************************
* @brief Response
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
namespace Command {
	const std::string COM_PING		= "ping";
	const std::string COM_ADD		= "add";
	const std::string COM_SEARCH	= "search";
	const std::string COM_TTL		= "ttl";
	const std::string COM_CHARGE	= "charge";
	const std::string COM_UPDATE	= "update";
	const std::string COM_REMOVE	= "remove";
	const std::string COM_COUNT		= "count";
	const std::string COM_MIRROR	= "mirror";
	const std::string COM_LEAVE		= "leave";
	const std::string COM_EXIT		= "exit";
}

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
	Privilege remove;
	Privilege change;
	Privilege charge;
};