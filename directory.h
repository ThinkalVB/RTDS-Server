#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <map>
#include "sp_entry.h"

class Directory
{
	static std::map<sourcePairV4, EntryV4*> V4EntryMap;		//!< STL map mapping V4 SourcePair address to the EntryV4 pointer
	static std::map<sourcePairV6, EntryV6*> V6EntryMap;		//!< STL map mapping V6 SourcePair address to the EntryV6 pointer

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
	static BaseEntry* _findEntry(const sourcePairV4&);
	static BaseEntry* _findEntry(const sourcePairV6&);
	static BaseEntry* _findEntry(const SourcePair&);
/*******************************************************************************************
* @brief Return true if cmdEntry have the privilege over entry to carry out the command
*
* @param[in] entry				The entry under command
* @param[in] cmdEntry			The commanding entry
* @return						True if have privilege
*
* @details
* These functions are not thread safe. [Need external locking for thread safety]
********************************************************************************************/
	static bool _havePrivilegeToCharge(BaseEntry*, BaseEntry*);
	static bool _havePrivilegeToRemove(BaseEntry*, BaseEntry*);
	static bool _havePrivilegeToChange(BaseEntry*, BaseEntry*);

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
	static BaseEntry* makeEntry(asio::ip::address_v4, unsigned short);
	static BaseEntry* makeEntry(asio::ip::address_v6, unsigned short);
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
	static BaseEntry* makeEntry(SourcePair&);

/*******************************************************************************************
* @brief Lock the entry for add [Lock must be released by calling releaseInsertionLock() method]
*
* @param[in] entry				The entry to be updated (self call)
* @param[out] insertionTocken	The insertion tocken for add command on SUCCESS
* @return						The Response to the operation
*
* @details
* Return SUCCESS and lock the entry if not in the directory and can be added.
* Return REDUDANT_DATA if the entry is already in the directory and can't be added.
********************************************************************************************/
	static Response addToDir(BaseEntry*, InsertionTocken&);
/*******************************************************************************************
* @brief Add the entry to the Directory (add itself)
*
* @param[in] sourcePair			Source pair address of the entry to be added
* @param[in] entry				The commanding entry
* @param[out] insertionTocken	The insertion tocken for add command on SUCCESS
* @return						The Response to the operation
*
* @details
* Return SUCCESS and lock the entry if not in the directory and can be added.
* Return REDUDANT_DATA if the entry is already in the directory and can't be added.
********************************************************************************************/
	static Response addToDir(SourcePair&, BaseEntry*, InsertionTocken&);
/*******************************************************************************************
* @brief Release the access lock of the entry and add the entry based on response
*
* @param[in] insertionTocken	The entry to be unlocked
* @param[in] reponse			The cumilative response for the add command
*
* @details
* If the response is SUCCESS then add the entry to the directory.
* If response is SUCCESS increment the entry count and update lastChargTime.
********************************************************************************************/
	static void releaseInsertionTocken(InsertionTocken&, Response&);

/*******************************************************************************************
* @brief Remove the entry from the directory (remove itself)
*
* @param[in] entry				The entry to be removed from the directory
* @return						The Response to the operation
*
* @details
* If the entry is in directory, remove it and return SUCCESS.
* Return NO_EXIST if the entry is not present in the directory.
********************************************************************************************/
	static Response removeFromDir(BaseEntry*);
/*******************************************************************************************
* @brief Remove the entry from the directory
*
* @param[in] sourcePair			The sourcePair address to the entry to be removed
* @param[in] cmdEntry			The entry commanding the operation
* @return						The Response to the operation
*
* @details
* If the entry is in directory, remove it and return SUCCESS.
* Return NO_EXIST if the entry is not present in the directory.
* Return NO_PRIVILAGE if the commanding entry lacks minimum permission.
********************************************************************************************/
	static Response removeFromDir(const SourcePair&, BaseEntry*);

/*******************************************************************************************
* @brief Get the Time to Live for that entry
*
* @param[in] entry				The entry for which the TTL is to be retrieved
* @param[out] ttl				The TTL for the entry
* @return						Return SUCCESS if the entry have a TTL
*
* @details
* Ensure the entry didn't expire and return SUCCESS if the entry have a TTL.
********************************************************************************************/
	static Response getTTL(BaseEntry*, short&);
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
* @brief Charge the entry (Charge itself ! no special effect)
*
* @param[in] entry				The entry to be charged in the directory
* @param[out] newTTL			The new TTL for the entry
* @return						Return SUCCESS if the charging is sucess
*
* @details
* Ensure the entry didn't expire and return the TTL of the entry.
********************************************************************************************/
	static Response charge(BaseEntry*, short&);
/*******************************************************************************************
* @brief Charge the entry
*
* @param[in] entry				The entry to be charged in the directory
* @param[in] cmdEntry			The entry commanding the operation
* @param[out] newTTL			The new TTL for the entry
* @return						Return SUCCESS if the charging is sucess
*
* @details
* Ensure the entry didn't expire and return the new TTL after charging the entry.
* Return NO_EXIST if the entry is not present in the directory.
* Return NO_PRIVILAGE if the commanding entry lacks minimum permission.
********************************************************************************************/
	static Response charge(const SourcePair&, BaseEntry*, short&);

/*******************************************************************************************
* @brief Lock the entry for updates [Lock must be released by calling releaseUpdateLock() method]
*
* @param[in] sourcePair			The entry to be updated in the directory
* @param[in] cmdEntry			The commanding entry
* @param[out] updateTocken		The updation tocken for update command on SUCCESS
* @return						Return SUCCESS if the event is updatable
*
* @details
* Return SUCCESS, updateTocken and lock the entry if it's in directory and have apt privilege.
* Return NO_EXIST if the entry is not present in the directory.
* Return NO_PRIVILAGE if the commanding entry lacks minimum permission.
********************************************************************************************/
	static Response getUpdateTocken(const SourcePair&, BaseEntry*, UpdateTocken&);
/*******************************************************************************************
* @brief Lock the entry for updates [Lock must be released by calling releaseUpdateLock() method]
*
* @param[in] entry				The entry to be updated (self call)
* @param[out] updateTocken		The updation tocken for update command on SUCCESS
* @return						Return SUCCESS if the event is updatable
*
* @details
* Return SUCCESS, updateTocken and lock the entry if it's in directory and have apt privilege.
* Return NO_EXIST if the entry is not present in the directory.
********************************************************************************************/
	static Response getUpdateTocken(BaseEntry*, UpdateTocken&);

/*******************************************************************************************
* @brief Release the access lock for the event
*
* @param[in] updateTocken		The entry to be unlocked
********************************************************************************************/
	static void releaseUpdateTocken(UpdateTocken&);
/*******************************************************************************************
* @brief Update the entry [Only use these functions after acquiring update lock on event]
*
* @param[in] entry				The entry to be updated in the directory
* @param[in] perm				Permission to be updated
********************************************************************************************/
	static Response update(UpdateTocken&, const Permission&);
/*******************************************************************************************
* @brief Update the entry [Only use these functions after acquiring update lock on event]
*
* @param[in] entry				The entry to be updated in the directory
* @param[in] desc				Description to be updated
********************************************************************************************/
	static void update(UpdateTocken&, const std::string_view&);

/*******************************************************************************************
* @brief Get the total number of entries in the directory
*
* @return						Return the total number of entries in the directory
********************************************************************************************/
	static int getEntryCount();
};

#endif