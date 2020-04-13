#pragma once
#include <cstdint>
#include <string>

/*******************************************************************************************
* @brief Enum class representing the levels of privilege
*
* @details
* LIBERAL_ENTRY		All Peers with any IP address and port number will have authority
* PROTECTED_ENTRY	All Peers with the same IP address will have authority
* RESTRICTED		Only peer with the same IP address and port number have authority
********************************************************************************************/
enum class Permission : uint8_t
{
	LIBERAL_ENTRY,
	PROTECTED_ENTRY,
	RESTRICTED_ENTRY
};

/*******************************************************************************************
* @brief Union class to store the Source Pair (Unique ID / address)
*
* @details
* The source port address is represented in network byte order
* V4SP				Source Port address (IPaddress [4Bytes]  + portNumber [2Bytes])
* V6SP				Source Port address (IPaddress [16Bytes] + portNumber [2Bytes])
********************************************************************************************/
union SourcePair
{
	uint8_t V4SP[6];
	uint8_t V6SP[18];
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
	const std::string CHOOSE_DIR	=	"choose_dir";
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
	const std::string COM_REGISTER	= "register";
	const std::string COM_SWITCH	= "switch";
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
	const std::string COM_DELETE	= "delete";
}