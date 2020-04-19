#pragma once
#include "cppcodec/base64_rfc4648.hpp"
#include "peer.h"

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
	static void _update(Peer&);

	template<typename sourcePairT>
	static void _print_ttl(Peer&, sourcePairT&);
	static void _ttl(Peer&);

/*******************************************************************************************
* @brief Extract the UID from the command line.
*
* @param[in] commandLine		The string view of the commandLine
* @param[out] element			Element extracted from the commandLine.
* @return						True if an element is found
*
* @details
* The element will be purged from the command line before returning a flag.
********************************************************************************************/
	static bool _extractElement(std::string_view&, std::string_view&);
/*******************************************************************************************
* @brief Find if the string is an UID
*
* @return						True if the strig view is an UID
********************************************************************************************/
	static bool _isUID(const std::string_view&);
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
	static bool _makeSourcePair(std::string_view&, std::string_view&, SourcePair&);
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
	static bool _validPortNumber(std::string_view&, unsigned short&);
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

template<typename sourcePairT>
inline void CmdInterpreter::_print_ttl(Peer& peer, sourcePairT& sourcePair)
{
	auto entry = Directory::findEntry(sourcePair);
	if (entry != nullptr)
	{
		peer.writeBuffer += RESP[(short)Response::SUCCESS] + " ";
		entry->printTTL(peer.writeBuffer);
	}
	else
		peer.writeBuffer += RESP[(short)Response::NO_EXIST];
}

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
