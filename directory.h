#pragma once
#include <map>
#include "sp_entry.h"

class Directory
{
	static std::map<sourcePairV4, EntryV4*> V4EntryMap;
	static std::map<sourcePairV6, EntryV6*> V6EntryMap;

	static std::mutex V4insertionLock;
	static std::mutex V6insertionLock;
	static std::mutex V4removalLock;
	static std::mutex V6removalLock;
public:
	static EntryV4* makeV4Entry(asio::ip::address_v4, unsigned short);
	static EntryV6* makeV6Entry(asio::ip::address_v6, unsigned short);
};

