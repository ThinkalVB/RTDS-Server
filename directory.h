#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <map>
#include "common.hpp"
#include "entry.h"

typedef std::map<SourcePairV4, Entry*> V4EntryMap;
typedef std::map<SourcePairV6, Entry*> V6EntryMap;

class Directory
{
	static V4EntryMap entryMapV4;					//!< STL map mapping V4 SourcePair address to the EntryV4 pointer
	static std::mutex v4InsLock;					//!< Lock this mutex before insertion
	static V6EntryMap entryMapV6;					//!< STL map mapping V6 SourcePair address to the EntryV6 pointer
	static std::mutex v6InsLock;					//!< Lock this mutex before insertion
	static std::atomic<std::size_t> _entryCount;	//!< Total number of entries in the directory		

	static const ResponseData _searchEntry(const SourcePairV4&);
	static const ResponseData _searchEntry(const SourcePairV6&);
	static const ResponseData _createV4Entry(const SPaddress&, const SPaddress&, const MutableData&);
	static const ResponseData _createV6Entry(const SPaddress&, const SPaddress&, const MutableData&);
	static const ResponseData _removeEntry(const SourcePairV4&, const SPaddress&);
	static const ResponseData _removeEntry(const SourcePairV6&, const SPaddress&);
	static const ResponseData _updateEntry(const SourcePairV4&, const SPaddress&, const MutableData&);
	static const ResponseData _updateEntry(const SourcePairV6&, const SPaddress&, const MutableData&);
	static const ResponseData _ttlEntry(const SourcePairV4&);
	static const ResponseData _ttlEntry(const SourcePairV6&);
	static const ResponseData _chargeEntry(const SourcePairV4&, const SPaddress&);
	static const ResponseData _chargeEntry(const SourcePairV6&, const SPaddress&);

public:
	static const ResponseData createEntry(const SPaddress&, const SPaddress&, const MutableData&);
	static const ResponseData removeEntry(const SPaddress&, const SPaddress&);
	static const ResponseData updateEntry(const SPaddress&, const SPaddress&, const MutableData&);
	static const ResponseData ttlEntry(const SPaddress&);
	static const ResponseData chargeEntry(const SPaddress&, const SPaddress&);
	static const ResponseData searchEntry(const SPaddress&);
	static void printEntryWith(std::string&, const MutableData&);

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
