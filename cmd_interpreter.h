#pragma once
#include "peer.h"

class CmdInterpreter
{
	static void s_ping(Peer&);
	static void s_count(Peer&);
	static void s_exit(Peer&);
	static void s_mirror(Peer&);
	static void s_leave(Peer&);

	static void _add(Peer&);
	static void _search(Peer&);
	static void _charge(Peer&);
	static void _ttl(Peer&);
	static void _remove(Peer&);
	static void _flush(Peer&);
	static void _update(Peer&);

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
	static void byteSwap(Data& portNumber);
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
