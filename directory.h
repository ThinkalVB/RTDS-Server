#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <map>
#include "common.hpp"
#include "entry.h"
#include "tockens.h"

class Directory
{
	static std::map<SourcePairV4, Entry*> V4EntryMap;		//!< STL map mapping V4 SourcePair address to the EntryV4 pointer
	static std::map<SourcePairV6, Entry*> V6EntryMap;		//!< STL map mapping V6 SourcePair address to the EntryV6 pointer

	static std::mutex V4insertionLock;						//!< Lock this mutex before searching and insertion into V4map
	static std::mutex V6insertionLock;						//!< Lock this mutex before searching and insertion into V6map

/*******************************************************************************************
* @brief Return a pointer to the base class
*
* @param[in] sourcePair			The sourcePair address
* @return						The pointer to the entry base class or nullptr
*
* @details
* Find the pointer to the Entry's base class if it exists in the map.
********************************************************************************************/
	static Entry* _findEntry(const SourcePairV4&);
	static Entry* _findEntry(const SourcePairV6&);
	static Entry* _findEntry(const SPAddress&);

public:
/*******************************************************************************************
* @brief Find an entry from the directory using it's SourcePair address
*
* @param[in] sourcePair			The SourcePair address of the entry
* @return						The BaseEntry pointer or nullptr
*
* @details
* If the entry is in the directory then return the pointer to it's BaseEntry.
* Return nullptr if the entry is not in the directory.
********************************************************************************************/
	static Entry* findEntry(const SPAddress&);
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
	static Entry* makeEntry(asio::ip::address_v4, unsigned short);
	static Entry* makeEntry(asio::ip::address_v6, unsigned short);
/*******************************************************************************************
* @brief Return a pointer to V4/V6 Entry for the given source pair address
*
* @param[in] sourcePair			The source pair address
* @return						The pointer to the V4/V6 Entry
*
* @details
* Make a sourcePair V4/V6 with the provided address and port number.
* Do insertion lock and check if an entry exists, yes then return it's pointer.
* No entry exists then generate EntryV4 dynamically, insert it to map and return it's pointer.
* Values of portNumber, UID, sourcePort V4/V6 address and IP4/IP6 address will be initialized.
********************************************************************************************/
	static Entry* makeEntry(SPAddress&);

/*******************************************************************************************
* @brief Get the total number of entries in the directory
*
* @return						Return the total number of entries in the directory
********************************************************************************************/
	static int getEntryCount();
/*******************************************************************************************
* @brief Wipe all the entries from the directory (dynamic deallocation)
********************************************************************************************/
	static void clearDirectory();
/*******************************************************************************************
* @brief Return True if the entry is in the directory
*
* @param[in] entry				The pointer to the entry
* @return						True if the entry is in directory
********************************************************************************************/
	static bool isInDirectory(Entry*);

	static Response insertEntry(InsertionTocken*, const MutableData&);
/*******************************************************************************************
* @brief Update the entry's permission and description
*
* @param[in] updateTocken		Tocken with entryPtr to be updated
* @param[in] data				Mutable data with new permission and description
* @return						SUCCESS on successfull updation
*
* @details
* SUCCESS on successfull updation. Return NO_PRIVILEGE if permission is not valid.
********************************************************************************************/
	static Response updateEntry(UpdateTocken*, const MutableData&);
/*******************************************************************************************
* @brief Flush the entries details through write buffer
*
* @param[in] writeBuffer		Buffer to which the entry details will be streamed
* @param[in] flushCount			Maximum number of entries to be flushed
*
* @details
* Write NO_EXIST if the directory is empty.
********************************************************************************************/
	static void flushEntries(std::string&, std::size_t);
/*******************************************************************************************
* @brief Remove the entry from directory
*
* @param[in] purgeTocken		The purge tocken for the remove command
********************************************************************************************/
	static void removeEntry(PurgeTocken*);
/*******************************************************************************************
* @brief Return the new TTL of the entry
*
* @param[in] chargeTocken		The charge tocken for the charging command
* @return						The new TTL after the charging process
********************************************************************************************/
	static short chargeEntry(ChargeTocken*);
/*******************************************************************************************
* @brief Return the Time To Live for that entry
*
* @param[in] entry				The pointer to the entry
* @return						Time To Live
********************************************************************************************/
	static short getTTL(Entry*);
/*******************************************************************************************
* @brief Print the brief info - IPversion, IPaddress, PortNumber
*
* @param[in] writeBuffer		Entry for which the brief is to be printed
* @param[out] writeBuffer		String buffer to which the data will be written.
********************************************************************************************/
	static void printBrief(Entry*, std::string&);
/*******************************************************************************************
* @brief Print the UID
*
* @param[in] writeBuffer		Entry for which the UID is to be printed
* @param[out] strBuffer			String buffer to which the data will be written.
********************************************************************************************/
	static void printUID(const Entry*, std::string&);
/*******************************************************************************************
* @brief Print the Version,UID, IP address, Port number, Permission and Description
*
* @param[in] writeBuffer		Entry for which the expanded form is to be printed
* @param[out] strBuffer			String buffer to which the data will be written.
********************************************************************************************/
	static void printExpand(Entry*, std::string&);
};

#endif