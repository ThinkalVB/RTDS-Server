#include "directory.h"
#include "cmd_interpreter.h"
#include "cppcodec/base64_rfc4648.hpp"

std::map<sourcePairV4, EntryV4*> Directory::V4EntryMap;
std::map<sourcePairV6, EntryV6*> Directory::V6EntryMap;

std::mutex Directory::V4insertionLock;
std::mutex Directory::V6insertionLock;
int Directory::entryCount = 0;

int Directory::getEntryCount()
{
	return entryCount;
}

EntryV4* Directory::makeEntry(asio::ip::address_v4 ipAdd, unsigned short portNum)
{
	sourcePairV4 sourcePair;
	CmdInterpreter::makeSourcePair(ipAdd, portNum, sourcePair);

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
			entry->sourcePair = sourcePair;
			V4EntryMap.insert(std::pair<sourcePairV4, EntryV4*>(sourcePair, entry));
			return entry;
		}
	}
}

EntryV6* Directory::makeEntry(asio::ip::address_v6 ipAdd, unsigned short portNum)
{
	sourcePairV6 sourcePair;
	CmdInterpreter::makeSourcePair(ipAdd, portNum, sourcePair);

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
			entry->sourcePair = sourcePair;
			V6EntryMap.insert(std::pair<sourcePairV6, EntryV6*>(sourcePair, entry));
			return entry;
		}
	}
}

EntryV4* Directory::findEntry(asio::ip::address_v4& ipAdd, unsigned short portNum)
{
	sourcePairV4 sourcePair;
	CmdInterpreter::makeSourcePair(ipAdd, portNum, sourcePair);
	auto entry = V4EntryMap.find(sourcePair);
	if (entry != V4EntryMap.end())
	{
		if (entry->second->isInDirectory && !entry->second->_haveExpired())
			return entry->second;
		else
			return nullptr;
	}
	else
		return nullptr;
}

EntryV6* Directory::findEntry(asio::ip::address_v6& ipAdd, unsigned short portNum)
{
	sourcePairV6 sourcePair;
	CmdInterpreter::makeSourcePair(ipAdd, portNum, sourcePair);
	auto entry = V6EntryMap.find(sourcePair);
	if (entry != V6EntryMap.end())
	{
		if (entry->second->isInDirectory || !entry->second->_haveExpired())
			return entry->second;
		else
			return nullptr;
	}
	else
		return nullptr;
}

/*
void Directory::addEntry(EntryV4* entry,EntryV4* CmdEntry)
{
	if (entry == CmdEntry)
	{
		entry->accessLock.lock();
		entry->permission.charge = Privilege::PROTECTED_ENTRY;
		entry->permission.change = Privilege::PROTECTED_ENTRY;
		entry->permission.remove = Privilege::PROTECTED_ENTRY;
		entry->accessLock.unlock();
	}
	else
	{
		std::lock_guard<std::mutex> lock(entry->accessLock);
		std::lock_guard<std::mutex> lock(CmdEntry->accessLock);

		if (!entry->isInDirectory)
		{
			entry->isInDirectory = true;
			entry->_chargeEntry();
			auto maxCmdEntryPrivilege = _maxPrivilege(entry, CmdEntry);
			if (maxCmdEntryPrivilege == Privilege::LIBERAL_ENTRY)
			{
				entry->permission.charge = Privilege::LIBERAL_ENTRY;
				entry->permission.change = Privilege::LIBERAL_ENTRY;
				entry->permission.remove = Privilege::LIBERAL_ENTRY;

			}
			else
			{
				entry->permission.charge = Privilege::PROTECTED_ENTRY;
				entry->permission.change = Privilege::PROTECTED_ENTRY;
				entry->permission.remove = Privilege::PROTECTED_ENTRY;
			}
		}

	}

}
*/