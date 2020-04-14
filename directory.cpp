#include "directory.h"
#include "cmd_interpreter.h"
#include "cppcodec/base64_rfc4648.hpp"

std::map<sourcePairV4, EntryV4*> Directory::V4EntryMap;
std::map<sourcePairV6, EntryV6*> Directory::V6EntryMap;

std::mutex Directory::V4insertionLock;
std::mutex Directory::V6insertionLock;
std::mutex Directory::V4removalLock;
std::mutex Directory::V6removalLock;

EntryV4* Directory::makeV4Entry(asio::ip::address_v4 ipAdd, unsigned short portNum)
{
	sourcePairV4 sourcePair;
	CmdInterpreter::makeSourcePairV4(ipAdd, portNum, sourcePair);

	{
		std::lock_guard<std::mutex> lock(V4insertionLock);
		auto entry = V4EntryMap.find(sourcePair);
		if (entry != V4EntryMap.end())
			return entry->second;
		else
		{
			auto entry = new EntryV4();
			entry->portNumber = std::to_string(portNum);
			entry->ipAddress = ipAdd.to_string();
			entry->UID = cppcodec::base64_rfc4648::encode(sourcePair);
			std::copy(std::begin(sourcePair), std::end(sourcePair), std::begin(entry->sourcePair.V4SP));
			return entry;
		}
	}
}

EntryV6* Directory::makeV6Entry(asio::ip::address_v6 ipAdd, unsigned short portNum)
{
	sourcePairV6 sourcePair;
	CmdInterpreter::makeSourcePairV6(ipAdd, portNum, sourcePair);

	{
		std::lock_guard<std::mutex> lock(V6insertionLock);
		auto entry = V6EntryMap.find(sourcePair);
		if (entry != V6EntryMap.end())
			return entry->second;
		else
		{
			auto entry = new EntryV6();
			entry->portNumber = std::to_string(portNum);
			entry->ipAddress = ipAdd.to_string();
			entry->UID = cppcodec::base64_rfc4648::encode(sourcePair);
			std::copy(std::begin(sourcePair), std::end(sourcePair), std::begin(entry->sourcePair.V6SP));
			return entry;
		}
	}
}
