#ifndef CMD_INTERPRETER_H
#define CMD_INTERPRETER_H

#include "cmd_element.h"
#include "common.hpp"
#include "mutable_data.h"
#include "spaddress.h"
#include "peer.h"

class CmdInterpreter
{
	static void s_ping(Peer&);
	static void s_count(Peer&);
	static void s_leave(Peer&);

	static void _mirror(Peer&);
	static void _add(Peer&);
	static void _search(Peer&);
	static void _charge(Peer&);
	static void _remove(Peer&);
	static void _ttl(Peer&);
	static void _update(Peer&);
public:
	static const std::string RESP[];			//!< All responses in string.
	static const std::string COMM[];			//!< All commands in string.
	static const char PRI[];				    //!< All privilage code in string.

/*******************************************************************************************
* @brief Process the commands from peer system
*
* @param[in] peer				The peer system from which the command is comming.
********************************************************************************************/
	static void processCommand(Peer&);
/*******************************************************************************************
* @brief Convert permission to string
*
* @param[in] permission			Permission
* @return						Permission as string
********************************************************************************************/
	static std::string toPermission(const Permission&);
/*******************************************************************************************
* @brief Convert the string to permission [use only after verfying by _isPermission()]
*
* @param[in] perm				The string view of the permission string
* @return						Permission
********************************************************************************************/
	static Permission toPermission(const std::string_view&);
/*******************************************************************************************
* @brief Convert the string to permission [use only after verfying by _isPermission()]
*
* @param[in] privilege			Character l,p or r for different privileges
********************************************************************************************/
	static Privilege toPrivilege(const char);
/*******************************************************************************************
* @brief Convert the string to it's equivalent intiger value
*
* @param[out] intValue			The intiger string
* @return						The intiger value
*
* @details
* This function won't catch hence the parameter must be an initger string.
********************************************************************************************/
	static int toIntiger(const std::string_view& intValue);
/*******************************************************************************************
* @brief Get the TTL for the privilege
*
* @param[in] maxPrivilege		The maximum privilege the entry have
* @return						Time to live for that particular privilege
********************************************************************************************/
	static unsigned short toInitialTTL(const Privilege);
/*******************************************************************************************
* @brief Convert the maxPrivilege to default permission
*
* @param[in] maxPriv			The maxPrivilege of targetSPA to cmdSPA
* @return						Default Permission
********************************************************************************************/
	static Permission toDefPermission(const Privilege);

/*******************************************************************************************
* @brief Check if the string is an Base64 encoded text
*
* @param[in] uid				The string view of the UID
* @return						True if the strig view is a base64 text
********************************************************************************************/
	static bool isBase64(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string is a valid description
*
* @param[in] desc				The string view of the description
* @return						True if the strig view is a description.
********************************************************************************************/
	static bool isDescription(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string is a policy permission
*
* @param[in] permission			The string view of the permission
* @return						True if the strig view is a permission
********************************************************************************************/
	static bool isPolicyPermission(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string is a valid permission
*
* @param[in] permission			The string view of the permission
* @return						True if the strig view is a permission
********************************************************************************************/
	static bool isPermission(const std::string_view&);
/*******************************************************************************************
* @brief Return true for valid port number
*
* @param[in] portNumStr			Port number as string view
* @param[out] portNum			A valid port number
* @return						True if the port number is valid
*
* @details
* Return true and a valid portNum if the string view is a valid port number. Else return false.
********************************************************************************************/
	static bool isPortNumber(const std::string_view&, unsigned short&);
/*******************************************************************************************
* @brief Return true if string is a valid intiger value
*
* @param[in] intValue			String view of the intiger value
* @return						True if string is a valid intiger value* @details
*
* @details
* Regex [0-9]{1,5} Range is 0 - 99999. No negative values are validated.
********************************************************************************************/
	static bool isIntiger(const std::string_view&);
/*******************************************************************************************
* @brief Return true if the Permission is valid for the maximim Privilege
*
* @param[in] perm				Permission seeking validity.
* @param[in] maxPriv			The maximum privilege cmdSPA have on targetSPA.
* @return						True if permission is valid.
********************************************************************************************/
	static bool isValid(const Permission&, const Privilege);
/*******************************************************************************************
* @brief Return true if the Permissions are comparable
*
* @param[in] perm1				Permission.
* @param[in] perm1				Permission.
* @return						True if permissions are comparable.
********************************************************************************************/
	static bool isComparable(const Permission&, const Permission&);

/*******************************************************************************************
* @brief Extract all command elements from the command line.
*
* @param[in] dataBuffer			Buffer that store the incoming data
* @param[in] cmdElement			Data structure to store the command elements
* @param[in] size				Number of characters received from the socket
* @return						True if command elements is found
*
* @details
* The command elements will be populated in the cmdElements.
* If their are more than 5 command elements, then the command line will be considered invalid.
* For this function to return true their must be atleast 1 command element present.
********************************************************************************************/
	static bool makeCmdElement(ReceiveBuffer&, CommandElement&, const std::size_t);
/*******************************************************************************************
* @brief Return true if a valid source pair is generated
*
* @param[in] ipAddrStr			String view of the ip address
* @param[out] spAddress			SourcePair as result
* @return						True if a source pair is generated
*
* @details
* Return true and a valid sourcePair if the string view for UID is valid.
********************************************************************************************/
	static bool makeSourcePair(const std::string_view&, SPaddress&);
/*******************************************************************************************
* @brief Return true if a valid source pair is generated
*
* @param[in] ipAddrStr			String view of the ip address
* @param[in] portNum			String view of the port number
* @param[out] spAddress			SourcePair as result
* @return						True if a source pair is generated
*
* @details
* Return true and a valid sourcePair if the string view for ipAddrStr and portNum are valid.
********************************************************************************************/
	static bool makeSourcePair(const std::string_view&, const std::string_view&, SPaddress&);

/*******************************************************************************************
* @brief Return mutable data from the commandElement
*
* @param[in] cmdElement			Command element holding the elements of the command
* @return						Mutable data
*
* @details
* The values used for generating the mutable data will be purged from the commandElement
********************************************************************************************/
	static const MutableData tryExtractMutData(CommandElement&);
/*******************************************************************************************
* @brief Return Policy from the commandElement
*
* @param[in] cmdElement			Command element holding the elements of the command
* @return						Policy (must ve verified with isValidPolicy() function)
*
* @details
* The values used for generating the Policy will be purged from the commandElement
********************************************************************************************/
	static const MutableData tryExtractPolicyMD(CommandElement&);
/*******************************************************************************************
* @brief Generate a sourcePair address from the peer command element if available
*
* @param[in] peer				Peer object with a valid command in it's commanding element.
* @param[out] sourcePair		SourcePair address if available.
*
* @details
* Generate a source pair if a valid ipAddress and port number is available.
* If IP and port number is not given, then spAddress remains unchanged.
********************************************************************************************/
	static void tryExtractSPA(CommandElement&, SPaddress&);

/*******************************************************************************************
* @brief Swap the bytes ( Little endian <---> Big endian )
*
* @param[out] Data				The data to be swapped.
********************************************************************************************/
	template <typename Data>
	static void byteSwap(Data&);
};

template <typename Data>
inline void CmdInterpreter::byteSwap(Data& portNumber)
{
	char* startIndex = static_cast<char*>((void*)&portNumber);
	char* endIndex = startIndex + sizeof(Data);
	std::reverse(startIndex, endIndex);
}

#endif