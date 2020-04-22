#pragma once
#include <map>
#include "sp_entry.h"

class Directory
{
	static std::map<sourcePairV4, EntryV4*> V4EntryMap;		//!< STL map mapping V4 SourcePair address to the EntryV4 pointer
	static std::map<sourcePairV6, EntryV6*> V6EntryMap;		//!< STL map mapping V6 SourcePair address to the EntryV6 pointer

	static std::mutex V4insertionLock;						//!< Lock this mutex before searching and insertion into V4map
	static std::mutex V6insertionLock;						//!< Lock this mutex before searching and insertion into V6map
	static int entryCount;

/*******************************************************************************************
* @brief Return a pointer to the base class
*
* @param[in] sourcePair			The sourcePair address
* @return						The pointer to the entry base class or nullptr
*
* @details
* Find the pointer to the Entry's base class if it exists in the map.
********************************************************************************************/
	static __base_entry* _findEntry(const sourcePairV4&);
	static __base_entry* _findEntry(const sourcePairV6&);
	static __base_entry* _findEntry(const SourcePair&);

public:
/*******************************************************************************************
* @brief Return a pointer to V4/V6 Entry for the given IPv4/IPv6 address and port number
*
* @param[in] ipAddr				The IPv4/IPv6 address
* @param[in] portNum			The port number
* @return						The pointer to the V4/V6 Entry
*
* @details
* Make a sourcePair V4/V6 with the provided address and port number.
* Do insertion lock and check if an entry exists, yes then return it's pointer.
* No entry exists then generate EntryV4 dynamically, insert it to map and return it's pointer.
* Values of portNumber, UID, sourcePort V4/V6 address and IP4/IP6 address will be initialized.
********************************************************************************************/
	static __base_entry* makeEntry(asio::ip::address_v4, unsigned short);
	static __base_entry* makeEntry(asio::ip::address_v6, unsigned short);
/*******************************************************************************************
* @brief Add the entry to the Directory (add itself)
*
* @param[in] entry				The entry to be added to the directory
* @return						The Response to the operation.
*
* @details
* Ensure the entry didn't expired and return SUCCESS if the entry is added to the directory.
* Return REDUDANT_DATA if the entry is already in the directory.
********************************************************************************************/
	static Response addToDir(__base_entry*);
/*******************************************************************************************
* @brief Remove the entry from the directory (remove itself)
*
* @param[in] entry				The entry to be added to the directory
* @return						The Response to the operation.
*
* @details
* If the entry is in directory, remove it and return SUCCESS.
* Return NO_EXIST if the entry is not present in the directory.
********************************************************************************************/
	static Response removeFromDir(__base_entry*);
	static Response removeFromDir(const SourcePair&, __base_entry*);




/*******************************************************************************************
* @brief Get the Time to Live for that entry
*
* @param[in] entry				The entry to be added to the directory
* @param[out] ttl				The TTL for the entry
* @return						Return SUCCESS if the entry have a TTL
*
* @details
* Ensure the entry didn't expired and return SUCCESS if the entry have a TTL.
********************************************************************************************/
	static Response getTTL(__base_entry*, short&);
/*******************************************************************************************
* @brief Get the Time to Live for that source pair
*
* @param[in] sourcePair			The source pair address to the entry
* @param[out] ttl				The TTL for the entry
* @return						Return SUCCESS if the entry have a TTL
*
* @details
* Search for the source pair address in maps and return TTL on SUCCESS.
********************************************************************************************/
	static Response getTTL(const SourcePair&, short&);
/*******************************************************************************************
* @brief Get the total number of entries in the directory
*
* @return						Return the total number of entries in the directory
********************************************************************************************/
	static int getEntryCount();
};
