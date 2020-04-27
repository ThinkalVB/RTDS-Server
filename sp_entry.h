#ifndef SP_ENTRY_H
#define SP_ENTRY_H

#include "common.hpp"
#include "cppcodec/base64_rfc4648.hpp"
#include <variant>
#include <boost/asio.hpp>

using namespace boost;

/*******************************************************************************************
 * @brief Base class for every entry into the directory.
 *
 * @details
 * Base class for the Entry for IPv4 and IPV6
 * The member functions of this class in not thread safe. Directory class must guarantee safety.
 ********************************************************************************************/
class BaseEntry
{
/*******************************************************************************************
* @brief Return the time passed after the last charge
*
* @return						Time in minutes, passed after last charge.
********************************************************************************************/
	short _tmAfterLastChrg();
/*******************************************************************************************
* @brief Turn the isInDirectory flag to false if the entry's TTL have expired
********************************************************************************************/
	void _doExpiryCheck();

protected:
	static const std::string VER[];				//!< All version code in string.
	static const char PRI[];				    //!< All privilage code in string.
	
	static int entryCount;						//!< Total entries in the directory.
	static BaseEntry* begin;					//!< Starting pointer to the entry list.
	static BaseEntry* end;						//!< Starting pointer to the entry list.
	static std::mutex entryTrainLock;			//!< Lock this mutex before inserting and removing from train.

	BaseEntry* next;							//!< Pointer to the next entry.
	BaseEntry* previous;						//!< Pointer to the previous entry.
	Version version;							//!< Version of the derived class IPV4 or IPV6.
	Permission permission;						//!< Level of privilage needed by the peer to execute commands.
	TTL timeToLive;								//!< Keep the time to live for this entry.

	bool iswithPeer = false;					//!< True if this entry is associated with a peer.
	bool isInDirectory = false;					//!< True if the entry is a directory entry.
	posix_time::ptime lastChargT;				//!< The time at which this entry was last charged.
	posix_time::ptime createdT;					//!< The time at which this entry was added to the directory.
			
	std::recursive_mutex accessLock;			//!< Lock these mutex when accessing or modfying data.
	std::string UID;							//!< Base 64 encoding of the source pair address.	
	std::string ipAddress;						//!< IPaddress associated with the entry.
	std::string portNumber;						//!< Port number associated with the entry.
	std::string description;					//!< Description associated with the entry.
/*******************************************************************************************
* @brief Get number of minutes after which the entry expires
*
* @return						Number of minutes to expiry
*
* @details
* Use this methord only if the entry is in the directory.
* [Not thread safe]
********************************************************************************************/
	short getTTL();
/*******************************************************************************************
* @brief Print the Expanded info - IPversion, UID, IPaddress, PortNumber, permission, description
*
* @param[out] strBuffer			String buffer to which the data will be written.
* [Not thread safe]
********************************************************************************************/
	void printExpand(std::string&);
/*******************************************************************************************
* @brief Return true if the entry is in directory
*
* @return						True if entry is in directory.
*
* @details
* Do expiry check before returning isInDirectory flag.
* [Not thread safe]
********************************************************************************************/
	bool inDirectory();
/*******************************************************************************************
* @brief Return the maximum privilage cmdEntry have on this entry
*
* @param[in] cmdEntry			The command issuing entry
* @return						Return the maximum privilage
*
* @details
* [Not thread safe]
********************************************************************************************/
	Privilege maxPrivilege(BaseEntry*);
/*******************************************************************************************
* @brief Add the entry to the directory
*
* @details
* Increment the entryCount, turn isInDirectory flag to true, update lastChargeT
* [Not thread safe]
********************************************************************************************/
	void addToDirectory(TTL);
/*******************************************************************************************
* @brief Print the Expanded info - IPversion, UID, IPaddress, PortNumber, permission, description
*
* @details
* Decrement the entryCount, turn isInDirectory flag to false
* [Not thread safe]
********************************************************************************************/
	void removeFromDirectory();
/*******************************************************************************************
* @brief Assign default permission to the entry
*
* @param[in] maxPrivilege		The maximum privilege commanding entry have on this entry
*
* @details
* PROTECTED_ENTRY if either protected or restricted, else LIBERAL_ENTRY
* [Not thread safe]
********************************************************************************************/
	void assignDefualtPermission(Privilege);

public:
/*******************************************************************************************
* @brief Print the UID
*
* @param[out] strBuffer			String buffer to which the data will be written.
********************************************************************************************/
	void printUID(std::string&);
/*******************************************************************************************
* @brief Print the brief info - IPversion, IPaddress, PortNumber
*
* @param[out] strBuffer			String buffer to which the data will be written.
********************************************************************************************/
	void printBrief(std::string&);
/*******************************************************************************************
* @brief Make this entry part of a peer
*
* @details
* Turn the isWithPeer to true which will prevent the directory from deleting this entry.
* When the entry is with peer it doesn't need charging. TTL is always RESTRICTED_TTL.
********************************************************************************************/
	void attachToPeer();
/*******************************************************************************************
* @brief Detach the entry from peer
*
* @details
* Turn the isWithPeer to false so that directory can delete this entry after TTL.
* Update the lastChargeT for accurate TTL calculation.
********************************************************************************************/
	void detachFromPeer();
/*******************************************************************************************
* @brief Get the version of the entry
*
* @return						Version type V4 or V6
********************************************************************************************/
	const Version& getVersion();
	friend class Directory;
};


template <typename SP, typename IPaddrT>
class SPentry : public BaseEntry
{
	SPentry(SP&, IPaddrT&, unsigned short);
	const SP sourcePair;
public:
/*******************************************************************************************
* @brief Return the maximum privilege the command issuing entry have
*
* @param[in] cmdEntry			The ptr to the command issuing Entry.
* @return						The maximum privilage the comEntry have on entry.
*
* @details
* If both IP address are not the same then - return Privilege::LIBERAL_ENTRY;
* Both have the same IP  version address - return Privilege::PROTECTED_ENTRY;
********************************************************************************************/
	Privilege maxPrivilege(BaseEntry*);
	friend class Directory;
};

/*******************************************************************************************
* @brief Construct SPentry of type EntryV4 or EntryV6
*
* @param[in] srcPair			SourcePair type V4 or V6
* @param[in] ipAdd				IP address IPv4 or IPv6
* @param[in] portNum			Port Number of the sourcePair
*
* @details
* Create a sourcePair of type V4 or V6 ( only use with directory ). Initialize values.
********************************************************************************************/
template <typename SP, typename IPaddrT>
SPentry<SP, IPaddrT>::SPentry(SP& srcPair, IPaddrT& ipAdd, unsigned short portNum) : sourcePair(srcPair)
{
	portNumber = std::to_string(portNum);
	ipAddress = ipAdd.to_string();
	UID = cppcodec::base64_rfc4648::encode(sourcePair);
	createdT = posix_time::second_clock::universal_time();
	description = "[]";

	if constexpr (std::is_same_v<IPaddrT, asio::ip::address_v4>)
		version = Version::V4;
	else
		version = Version::V6;
}

template<typename SP, typename IPaddrT>
inline Privilege SPentry<SP, IPaddrT>::maxPrivilege(BaseEntry* cmdEntry)
{
	if (version == cmdEntry->getVersion())
	{
		auto cmdEntrySP = (void*)&((SPentry*)cmdEntry)->sourcePair[0];
		if (std::memcmp((void*)&sourcePair[0], cmdEntrySP, sourcePair.size() - 2) == 0)
			return Privilege::PROTECTED_ENTRY;
		else
			return Privilege::LIBERAL_ENTRY;
	}
	else
		return Privilege::LIBERAL_ENTRY;
}

typedef SPentry<SourcePairV4, asio::ip::address_v4> EntryV4;
typedef SPentry<SourcePairV6, asio::ip::address_v6> EntryV6;
/*******************************************************************************************
 * @brief Union to hold the poiter to either EntryV4 or EntryV6
 ********************************************************************************************/
union Entry
{
	BaseEntry* EvB;
	EntryV4* Ev4;
	EntryV6* Ev6;
};

struct UpdateTocken
{
private:
	BaseEntry* EvB;
	Privilege maxPrivilege;
public:
	BaseEntry* entry();
	friend class Directory;
};
typedef UpdateTocken InsertionTocken;

/*******************************************************************************************
 * @brief Struct to hold both SourcePairV4 and SourcePairV6
 ********************************************************************************************/
struct SourcePair
{
	union __source_pair
	{
		SourcePairV4 V4;
		IPVersion4 IPV4;
		SourcePairV6 V6;
		IPVersion6 IPV6;
	}SP;
	
	unsigned short portNumber();
	Version version;
};

/*******************************************************************************************
 * @brief Struct to hold individual command elements
 *
* @details
* This is an optimized container and shoudn't be used for any other purpose than populating elements.
* Before using pop_front(),push_back(), peek() and peek_next() explicit bound checks must be done.
* Populating the container must be done in a single stretch using push_back().
* After populating the container, call reset_for_read() to start reading.
* Before begining the push_back() streak call reset() once.
 ********************************************************************************************/
struct CommandElement
{
private:
	std::string_view element[5];
	std::size_t _size;
	std::size_t _index;
public:
	const std::size_t& size();
	void reset_for_read();
	void reset();

	const std::string_view& pop_front();
	void pop_front(int);
	const std::string_view& peek();
	const std::string_view& peek_next();
	void push_back(std::string_view);
};

#endif