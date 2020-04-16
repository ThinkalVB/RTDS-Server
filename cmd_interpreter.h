#pragma once
#include "peer.h"

class CmdInterpreter
{
	template<typename EntryPtr>
	static void __ping(Peer&, EntryPtr*);
	static void _ping(Peer&);

	template<typename EntryPtr>
	static void __add(Peer& peer, EntryPtr* entry);
public:
	static const std::string RESP[];
	static const std::string COMM[];
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

template<typename EntryPtr>
inline void CmdInterpreter::__ping(Peer& peer, EntryPtr* entry)
{
	peer.writeBuffer += RESP[(short)Response::SUCCESS] + " ";
	peer.writeBuffer += entry->versionID + " ";
	peer.writeBuffer += entry->ipAddress + " ";
	peer.writeBuffer += entry->portNumber;
}

template<typename EntryPtr>
inline void CmdInterpreter::__add(Peer& peer, EntryPtr* entry)
{
	auto response = Directory::addEntry(entry);
	std::lock_guard<std::mutex> lock(entry->accessLock);
	peer.writeBuffer += RESP[(short)response] + " ";
	peer.writeBuffer += entry->UID;
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
