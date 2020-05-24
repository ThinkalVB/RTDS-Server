#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <map>
#include "common.hpp"
#include "entry.h"

typedef std::map<SourcePairV4, Entry*> V4EntryMap;
typedef std::map<SourcePairV6, Entry*> V6EntryMap;


class Directory
{
	static V4EntryMap entryMapV4;				//!< STL map mapping V4 SourcePair address to the EntryV4 pointer
	static V6EntryMap entryMapV6;				//!< STL map mapping V6 SourcePair address to the EntryV6 pointer

	static std::mutex v4MapAccess;				//!< Lock this mutex before searching and insertion into V4map
	static std::mutex v6MapAccess;				//!< Lock this mutex before searching and insertion into V6map

	static ResponsePair _searchEntry(const SourcePairV4&);
	static ResponsePair _searchEntry(const SourcePairV6&);
	static void _searchV4Entry(const Policy&, std::string&);
	static void _searchV6Entry(const Policy&, std::string&);


	static ResponsePair _createV4Entry(const SPaddress&, const MutableData&, const Privilege);
	static ResponsePair _createV6Entry(const SPaddress&, const MutableData&, const Privilege);

	static ResponsePair _removeEntry(const SourcePairV4&, const SPaddress&);
	static ResponsePair _removeEntry(const SourcePairV6&, const SPaddress&);

	static ResponsePair _updateEntry(const SourcePairV4&, const SPaddress&, const MutableData&);
	static ResponsePair _updateEntry(const SourcePairV6&, const SPaddress&, const MutableData&);

	static ResponseTTL _ttlEntry(const SourcePairV4&);
	static ResponseTTL _ttlEntry(const SourcePairV6&);

	static ResponseTTL _chargeEntry(const SourcePairV4&, const SPaddress&);
	static ResponseTTL _chargeEntry(const SourcePairV6&, const SPaddress&);

public:
	static ResponsePair createEntry(const SPaddress&, const SPaddress&, const MutableData&);
	static ResponsePair removeEntry(const SPaddress&, const SPaddress&);
	static ResponsePair updateEntry(const SPaddress&, const SPaddress&, const MutableData&);
	static ResponseTTL ttlEntry(const SPaddress&);
	static ResponseTTL chargeEntry(const SPaddress&, const SPaddress&);
	static ResponsePair searchEntry(const SPaddress&);
	static void searchEntry(const Policy&, std::string&);

/*******************************************************************************************
* @brief Get the total number of entries in the Directory
*
* @return						Return the total number of entries in the directory
* [Not thread safe]
********************************************************************************************/
	static std::size_t entryCount();
/*******************************************************************************************
* @brief Wipe all the entries from the directory (dynamic deallocation)
********************************************************************************************/
	static void clearDirectory();
};
#endif
