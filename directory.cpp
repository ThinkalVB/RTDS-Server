#include "directory.h"

std::map<SourcePairV4, EntryV4*> Directory::V4EntryMap;
std::map<SourcePairV6, EntryV6*> Directory::V6EntryMap;

std::mutex Directory::V4insertionLock;
std::mutex Directory::V6insertionLock;

BaseEntry* Directory::_findEntry(const SourcePairV4& sourcePair)
{
	auto entry = V4EntryMap.find(sourcePair);
	if (entry != V4EntryMap.end())
		return entry->second;
	else 
		return nullptr;
}

BaseEntry* Directory::_findEntry(const SourcePairV6& sourcePair)
{
	auto entry = V6EntryMap.find(sourcePair);
	if (entry != V6EntryMap.end())
		return entry->second;
	else
		return nullptr;
}

BaseEntry* Directory::_findEntry(const SourcePair& sourcePair)
{
	if (sourcePair.version == Version::V4)
		return _findEntry(sourcePair.SP.V4);
	else
		return _findEntry(sourcePair.SP.V6);
}

BaseEntry* Directory::findEntry(const SourcePair& sourcePair)
{
	auto entry = _findEntry(sourcePair);
	if (entry != nullptr && entry->inDirectory())
		return entry;
	else
		return nullptr;	
}


BaseEntry* Directory::makeEntry(asio::ip::address_v4 ipAdd, unsigned short portNum)
{
	SourcePairV4 sourcePair;
	CmdInterpreter::makeSourcePair(ipAdd, portNum, sourcePair);

	std::lock_guard<std::mutex> lock(V4insertionLock);
	auto entryPtr = _findEntry(sourcePair);
	if (entryPtr == nullptr)
	{
		entryPtr = new EntryV4(sourcePair, ipAdd, portNum);
		V4EntryMap.insert(std::pair<SourcePairV4, EntryV4*>(sourcePair, (EntryV4*)entryPtr));
	}
	return entryPtr;
}

BaseEntry* Directory::makeEntry(asio::ip::address_v6 ipAdd, unsigned short portNum)
{
	SourcePairV6 sourcePair;
	CmdInterpreter::makeSourcePair(ipAdd, portNum, sourcePair);

	std::lock_guard<std::mutex> lock(V6insertionLock);
	auto entryPtr = _findEntry(sourcePair);
	if (entryPtr == nullptr)
	{
		entryPtr = new EntryV6(sourcePair, ipAdd, portNum);
		V6EntryMap.insert(std::pair<SourcePairV6, EntryV6*>(sourcePair, (EntryV6*)entryPtr));
	}
	return entryPtr;
}

BaseEntry* Directory::makeEntry(SourcePair& sourcePair)
{
	auto entry = _findEntry(sourcePair);
	if (entry != nullptr)
		return entry;
	else
	{
		if (sourcePair.version == Version::V4)
			entry = makeEntry(asio::ip::make_address_v4(sourcePair.SP.IPV4), sourcePair.portNumber());
		else
			entry = makeEntry(asio::ip::make_address_v6(sourcePair.SP.IPV6), sourcePair.portNumber());
	}
	return entry;
}


int Directory::getEntryCount()
{
	std::lock_guard<std::mutex> lock1(V4insertionLock);
	std::lock_guard<std::mutex> lock2(V6insertionLock);
	return BaseEntry::entryCount;
}

void Directory::clearDirectory()
{
	std::map<SourcePairV4, EntryV4*>::iterator itV4 = V4EntryMap.begin();
	while (itV4 != V4EntryMap.end())
	{
		delete (EntryV4*)itV4->second;
		itV4++;
	}
	V4EntryMap.clear();

	std::map<SourcePairV6, EntryV6*>::iterator itV6 = V6EntryMap.begin();
	while (itV6 != V6EntryMap.end())
	{
		delete (EntryV6*)itV6->second;
		itV6++;
	}
	V6EntryMap.clear();
}

bool Directory::isInDirectory(BaseEntry* entry)
{
	if (entry != nullptr && entry->inDirectory())
		return true;
	else
		return false;
}

Response Directory::insertEntry(InsertionTocken* insertionTocken, const MutableData& data)
{
	auto entry = insertionTocken->EvB;
	if (data.havePermission)
	{
		if (insertionTocken->haveValid(data.permission))
			entry->permission = data.permission;
		else
			return Response::NO_PRIVILAGE;
	}
	else
		entry->assignDefualtPermission(insertionTocken->maxPrivilege);

	if (data.haveDescription)
		entry->description = data.description;

	entry->addToDirectory(CmdInterpreter::toTTL(insertionTocken->maxPrivilege));
	return Response::SUCCESS;
}

Response Directory::updateEntry(UpdateTocken* updateTocken, const MutableData& data)
{
	auto entry = updateTocken->EvB;
	if (data.havePermission)
	{
		if (updateTocken->haveValid(data.permission))
			entry->permission = data.permission;
		else
			return Response::NO_PRIVILAGE;
	}

	if (data.haveDescription)
		entry->description = data.description;
	return Response::SUCCESS;
}

void Directory::flushEntries(std::string& writeBuffer, std::size_t flushCount)
{
	if (BaseEntry::begin == nullptr)
		writeBuffer += CmdInterpreter::RESP[(short)Response::NO_EXIST];
	else
	{
		auto entryPtr = BaseEntry::begin;
		while (entryPtr != nullptr && flushCount != 0)
		{
			std::lock_guard<std::mutex> lock(entryPtr->accessLock);
			entryPtr->printExpand(writeBuffer);
			entryPtr = entryPtr->next;
			flushCount--;
			if (entryPtr != nullptr)
				writeBuffer += '\x1f';		//!< Unit separator
		}
	}
}

void Directory::removeEntry(PurgeTocken* purgeTocken)
{
	auto entry = purgeTocken->EvB;
	entry->removeFromDirectory();
}

short Directory::chargeEntry(ChargeTocken* chargeTocken)
{
	auto entry = chargeTocken->EvB;
	auto newTTL = CmdInterpreter::toTTL(chargeTocken->maxPrivilege);
	return entry->charge(newTTL);
}

short Directory::getTTL(BaseEntry* entry)
{
	std::lock_guard<std::mutex> lock(entry->accessLock);
	return entry->getTTL();
}

void Directory::printBrief(BaseEntry* entry, std::string& writeBuffer)
{
	entry->printBrief(writeBuffer);
}

void Directory::printUID(const BaseEntry* entry, std::string& writeBuffer)
{
	writeBuffer += entry->UID;
}
