#ifndef CMD_INTERPRETER_H
#define CMD_INTERPRETER_H

#include "cmd_element.h"
#include "common.hpp"
#include "tockens.h"
#include "peer.h"
#include <string>

class CmdInterpreter
{
	static void s_ping(Entry*, std::string&);
	static void s_count(std::string&);
	static void s_mirror(Peer&);
	static void s_leave(Peer&);

	static void _add(Peer&);
	static void _search(Entry*, CommandElement&, std::string&);
	static void _charge(Entry*, CommandElement&, std::string&);
	static void _remove(Peer&);
	static void _flush(CommandElement&, std::string&);
	static void _ttl(Entry*, CommandElement&, std::string&);
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
	static Privilege toPrivilege(const char&);
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
	static TTL toTTL(Privilege);

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
	static bool makeCmdElement(ReceiveBuffer&, CommandElement&, std::size_t);
/*******************************************************************************************
* @brief Return true if a valid source pair is generated
*
* @param[in] ipAddrStr			String view of the ip address
* @param[in] portNum			String view of the port number
* @param[out] sourcePair		SourcePair as result
* @return						True if a source pair is generated
*
* @details
* Return true and a valid sourcePair if the string view for ipAddrStr and portNum are valid.
********************************************************************************************/
	static bool makeSourcePair(const std::string_view&, const std::string_view&, SPAddress&);
/*******************************************************************************************
* @brief Return true if a valid source pair is generated
*
* @param[in] ipAddrStr			String view of the ip address
* @param[out] sourcePair		SourcePair as result
* @return						True if a source pair is generated
*
* @details
* Return true and a valid sourcePair if the string view for UID is valid.
********************************************************************************************/
	static bool makeSourcePair(const std::string_view&, SPAddress&);


/*******************************************************************************************
* @brief Return portNumber from the sourcePair address
*
* @param[out] spa				SourcePair address
* @return						Port number from sourcePair address
********************************************************************************************/
	static unsigned short portNumber(const SPAddress&);


/*******************************************************************************************
* @brief Return mutable data from the commandElement
*
* @param[in] cmdElement			Command element holding the elements of the command
* @return						Mutable data
*
* @details
* The values used for generating the mutable data will be purged from the commandElement
********************************************************************************************/
	static const MutableData extractMutableData(CommandElement&);
/*******************************************************************************************
* @brief Extract flush count from the command line
*
* @param[in] cmdElement			Command element holding the elements of the command
* @param[out] flushCount		Number of elements to be flushed from directory
*
* @details
* The values used for generating the flushCount will be purged from the cmdElement.
********************************************************************************************/
	static void extractFlushCount(CommandElement&, std::size_t&);
/*******************************************************************************************
* @brief Return true if a valid source pair is generated
*
* @param[in] cmdElement			Command element holding the elements of the command
* @param[out] sourcePair		SourcePair as result
* @return						True if a source pair is generated
*
* @details
* Return true and a valid sourcePair if a valid source pair is found.
* The values used for generating the source pair will be purged from the cmdElement.
********************************************************************************************/
	static bool extractSourcePair(CommandElement&, SPAddress&);
/*******************************************************************************************
* @brief Extract sourcePair address and find corresponding base entry pointer
*
* @param[in] entry				Entry requesting extraction
* @param[in] cmdElement			Command element from which sourcePair is extracted
* @return						Pointer to the BaseEntry pointer
*
* @details
* If a sourcePair is found which is not in directory then return nullptr.
* If a sourcePair is found which is in directory then return it's BaseEntry pointer.
* If no sourcePair address is found then assume the BaseEntry to be itself.
********************************************************************************************/
	static Entry* extractBaseEntry(Entry*, CommandElement&);

/*******************************************************************************************
* @brief Swap the bytes ( Little endian <---> Big endian )
*
* @param[out] Data				The data to be swapped.
********************************************************************************************/
	template <typename Data>
	static void byteSwap(Data&);
/*******************************************************************************************
* @brief Make sourcePair address from IP address and portNumber
*
* @param[in] ipAddress			Ipv4 or Ipv6 address
* @param[in] portNum			Port Number
* @param[out] sourcePair		Return the source pair address
*
* @details
* Make source pair address in network byte order. For little endian systems swap bytes for port number.
********************************************************************************************/
	template <typename IPaddress, typename sourcePair>
	static void makeSourcePair(const IPaddress&, unsigned short, sourcePair&);
};


template <typename Data>
inline void CmdInterpreter::byteSwap(Data& portNumber)
{
	char* startIndex = static_cast<char*>((void*)&portNumber);
	char* endIndex = startIndex + sizeof(Data);
	std::reverse(startIndex, endIndex);
}

template<typename IPaddress, typename sourcePair>
inline void CmdInterpreter::makeSourcePair(const IPaddress& ipAddress, unsigned short portNum, sourcePair& sourcePair)
{
	auto ipBin = ipAddress.to_bytes();
	memcpy(&sourcePair[0], &ipBin[0], sourcePair.size() - 2);

	#ifdef BOOST_ENDIAN_LITTLE_BYTE
	byteSwap(portNum);
	#endif
	memcpy(&sourcePair[sourcePair.size() - 2], &portNum, 2);
}
#endif