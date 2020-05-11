#include "directory.h"
#include "cmd_interpreter.h"

std::map<SourcePairV4, Entry*> Directory::V4EntryMap;
std::map<SourcePairV6, Entry*> Directory::V6EntryMap;

std::mutex Directory::V4insertionLock;
std::mutex Directory::V6insertionLock;

Entry* Directory::_findEntry(const SourcePairV4& sourcePair)
{
	auto entry = V4EntryMap.find(sourcePair);
	if (entry != V4EntryMap.end())
		return entry->second;
	else 
		return nullptr;
}

Entry* Directory::_findEntry(const SourcePairV6& sourcePair)
{
	auto entry = V6EntryMap.find(sourcePair);
	if (entry != V6EntryMap.end())
		return entry->second;
	else
		return nullptr;
}

Entry* Directory::_findEntry(const SPAddress& sourcePair)
{
	if (sourcePair.version == Version::V4)
		return _findEntry(sourcePair.SPA.V4);
	else
		return _findEntry(sourcePair.SPA.V6);
}

Entry* Directory::findEntry(const SPAddress& sourcePair)
{
	auto entry = _findEntry(sourcePair);
	if (entry != nullptr && entry->inDirectory())
		return entry;
	else
		return nullptr;	
}


Entry* Directory::makeEntry(asio::ip::address_v4 ipAdd, unsigned short portNum)
{
	SourcePairV4 sourcePair;
	CmdInterpreter::makeSourcePair(ipAdd, portNum, sourcePair);

	std::lock_guard<std::mutex> lock(V4insertionLock);
	auto entryPtr = _findEntry(sourcePair);
	if (entryPtr == nullptr)
	{
		entryPtr = new Entry(sourcePair, ipAdd, portNum);
		V4EntryMap.insert(std::pair<SourcePairV4, Entry*>(sourcePair, entryPtr));
	}
	return entryPtr;
}

Entry* Directory::makeEntry(asio::ip::address_v6 ipAdd, unsigned short portNum)
{
	SourcePairV6 sourcePair;
	CmdInterpreter::makeSourcePair(ipAdd, portNum, sourcePair);

	std::lock_guard<std::mutex> lock(V6insertionLock);
	auto entryPtr = _findEntry(sourcePair);
	if (entryPtr == nullptr)
	{
		entryPtr = new Entry(sourcePair, ipAdd, portNum);
		V6EntryMap.insert(std::pair<SourcePairV6, Entry*>(sourcePair, entryPtr));
	}
	return entryPtr;
}

Entry* Directory::makeEntry(SPAddress& sourcePair)
{
	auto entry = _findEntry(sourcePair);
	if (entry != nullptr)
		return entry;
	else
	{
		if (sourcePair.version == Version::V4)
			entry = makeEntry(asio::ip::make_address_v4(sourcePair.SPA.IPV4), CmdInterpreter::portNumber(sourcePair));
		else
			entry = makeEntry(asio::ip::make_address_v6(sourcePair.SPA.IPV6), CmdInterpreter::portNumber(sourcePair));
	}
	return entry;
}


int Directory::getEntryCount()
{
	std::lock_guard<std::mutex> lock1(V4insertionLock);
	std::lock_guard<std::mutex> lock2(V6insertionLock);
	return Entry::dllController.size();
}

void Directory::clearDirectory()
{
	std::map<SourcePairV4, Entry*>::iterator itV4 = V4EntryMap.begin();
	while (itV4 != V4EntryMap.end())
	{
		delete itV4->second;
		itV4++;
	}
	V4EntryMap.clear();

	std::map<SourcePairV6, Entry*>::iterator itV6 = V6EntryMap.begin();
	while (itV6 != V6EntryMap.end())
	{
		delete itV6->second;
		itV6++;
	}
	V6EntryMap.clear();
}

bool Directory::isInDirectory(Entry* entry)
{
	if (entry != nullptr && entry->inDirectory())
		return true;
	else
		return false;
}

Response Directory::insertEntry(InsertionTocken* insertionTocken, const MutableData& data)
{
	auto entry = insertionTocken->EvB;
	if (data._havePermission)
	{
		if (insertionTocken->haveValid(data._permission))
			entry->permission = data._permission;
		else
			return Response::NO_PRIVILAGE;
	}
	else
		entry->assignDefualtPermission(insertionTocken->maxPrivilege);

	if (data._haveDescription)
		entry->description = data._description;

	entry->addToDirectory(CmdInterpreter::toTTL(insertionTocken->maxPrivilege));
	return Response::SUCCESS;
}

Response Directory::updateEntry(UpdateTocken* updateTocken, const MutableData& data)
{
	auto entry = updateTocken->EvB;
	if (data._havePermission)
	{
		if (updateTocken->haveValid(data._permission))
			entry->permission = data._permission;
		else
			return Response::NO_PRIVILAGE;
	}

	if (data._haveDescription)
		entry->description = data._description;
	return Response::SUCCESS;
}

void Directory::flushEntries(std::string& writeBuffer, std::size_t flushCount)
{
	if (Entry::dllController.begin() == nullptr)
		writeBuffer += CmdInterpreter::RESP[(short)Response::NO_EXIST];
	else
	{
		auto entryPtr = Entry::dllController.begin();
		while (entryPtr != nullptr && flushCount != 0)
		{
			printExpand(entryPtr, writeBuffer);
			entryPtr = entryPtr->dllNode.next();
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

short Directory::getTTL(Entry* entry)
{
	std::lock_guard<std::mutex> lock(entry->accessLock);
	return entry->getTTL();
}

void Directory::printBrief(Entry* entry, std::string& writeBuffer)
{
	entry->printBrief(writeBuffer);
}

void Directory::printUID(const Entry* entry, std::string& writeBuffer)
{
	writeBuffer += entry->UID;
}

void Directory::printExpand(Entry* entry, std::string& writeBuffer)
{
	std::lock_guard<std::mutex> lock(entry->accessLock);
	entry->printExpand(writeBuffer);
}
