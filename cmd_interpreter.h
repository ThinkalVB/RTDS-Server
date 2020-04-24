#pragma once
#include "peer.h"
#include "sp_entry.h"

class CmdInterpreter
{
	static void s_ping(Peer&);
	static void s_count(Peer&);
	static void s_mirror(Peer&);
	static void s_leave(Peer&);

	static void _add(Peer&);
	static void _search(Peer&);
	static void _charge(Peer&);
	static void _remove(Peer&);
	static void _flush(Peer&);
	static void _ttl(Peer&);
	static void _update(Peer&);

/*******************************************************************************************
* @brief Check if the string is an Base64 encoded text
*
* @return						True if the strig view is a base64 text
********************************************************************************************/
	static bool _isBase64(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string is a valid description
*
* @return						True if the strig view is a description.
********************************************************************************************/
	static bool _isDescription(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string is a valid permission
*
* @return						True if the strig view is a permission.
********************************************************************************************/
	static bool _isPermission(const std::string_view&);
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
	static bool _validPortNumber(const std::string_view&, unsigned short&);
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
	static bool _makeSourcePair(const std::string_view&, const std::string_view&, SourcePair&);
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
	static bool _makeSourcePair(const std::string_view&, SourcePair&);

public:
	static const std::string RESP[];			//!< All responses in string.
	static const std::string COMM[];			//!< All commands in string.
/*******************************************************************************************
* @brief Process the commands from peer system
*
* @param[in] peer				The peer system from which the command is comming.
********************************************************************************************/
	static void processCommand(Peer&);
/*******************************************************************************************
* @brief Extract all command elements from the command line.
*
* @param[in] peer				The peer for which the command elements are to be generated
* @param[out] size				Number of characters received from process data
* @return						True if command elements is found
*
* @details
* The command elements will be populated in the cmdElements.
* If their are more than 5 command elements, then the command line will be considered invalid.
* For this function to return true their must be atleast 1 command element present.
********************************************************************************************/
	static bool populateElement(Peer&, const size_t&);

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
	template <typename IPaddress,typename sourcePair>
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
