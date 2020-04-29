#include "directory.h"
#include "cmd_interpreter.h"

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


bool Directory::_havePrivilegeToCharge(BaseEntry* entry, BaseEntry* cmdEntry)
{
	auto maxPrivilege = entry->maxPrivilege(cmdEntry);
	if (maxPrivilege >= entry->permission.charge)
		return true;
	else
		return false;
}

bool Directory::_havePrivilegeToRemove(BaseEntry* entry, BaseEntry* cmdEntry)
{
	auto maxPrivilege = entry->maxPrivilege(cmdEntry);
	if (maxPrivilege >= entry->permission.remove)
		return true;
	else
		return false;
}

bool Directory::_havePrivilegeToChange(BaseEntry* entry, BaseEntry* cmdEntry)
{
	auto maxPrivilege = entry->maxPrivilege(cmdEntry);
	if (maxPrivilege >= entry->permission.change)
		return true;
	else
		return false;
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


Response Directory::addToDir(BaseEntry* entry, InsertionTocken& updateTocken)
{
	entry->accessLock.lock();
	updateTocken.EvB = entry;
	if (!entry->inDirectory())
	{
		updateTocken.maxPrivilege = Privilege::RESTRICTED_ENTRY;
		entry->assignDefualtPermission(Privilege::RESTRICTED_ENTRY);
		return Response::SUCCESS;
	}
	else
	{
		entry->accessLock.unlock();
		return Response::REDUDANT_DATA;
	}
}

Response Directory::addToDir(SourcePair& sourcePair, BaseEntry* cmdEntry, InsertionTocken& insertionTocken)
{
	auto entry = makeEntry(sourcePair);
	Response response;

	if (entry == cmdEntry)
		return addToDir(entry, insertionTocken);
	else
	{
		entry->accessLock.lock();
		insertionTocken.EvB = entry;
		if (!entry->inDirectory())
		{
			insertionTocken.maxPrivilege = entry->maxPrivilege(cmdEntry);
			entry->assignDefualtPermission(insertionTocken.maxPrivilege);
			return Response::SUCCESS;
		}
		else
			response = Response::REDUDANT_DATA;
	}

	entry->accessLock.unlock();
	return response;
}

void Directory::releaseInsertionTocken(InsertionTocken& insertionTocken, Response& reponse)
{
	if (reponse == Response::SUCCESS)
		insertionTocken.EvB->addToDirectory(CmdInterpreter::toTTL(insertionTocken.maxPrivilege));
	insertionTocken.EvB->accessLock.unlock();
}


Response Directory::removeFromDir(BaseEntry* entry)
{
	std::lock_guard<std::recursive_mutex> lock(entry->accessLock);
	if (entry->inDirectory())
	{
		entry->removeFromDirectory();
		return Response::SUCCESS;
	}
	else
		return Response::NO_EXIST;
}

Response Directory::removeFromDir(const SourcePair& sourcePair, BaseEntry* cmdEntry)
{
	auto entry = _findEntry(sourcePair);
	if (entry == nullptr)
		return Response::NO_EXIST;

	if (entry == cmdEntry)
		return removeFromDir(entry);
	else
	{
		std::lock_guard<std::recursive_mutex> lock(entry->accessLock);
		if (_havePrivilegeToRemove(entry, cmdEntry))
			return removeFromDir(entry);
		else
			return Response::NO_PRIVILAGE;
	}
}
	

Response Directory::getTTL(BaseEntry* entry, short& ttl)
{
	std::lock_guard<std::recursive_mutex> lock(entry->accessLock);
	if (entry->inDirectory())
	{
		ttl = entry->getTTL();
		return Response::SUCCESS;
	}
	else
		return Response::NO_EXIST;
}

Response Directory::getTTL(const SourcePair& sourcePair, short& ttl)
{
	auto entry = _findEntry(sourcePair);	
	if (entry == nullptr)
		return Response::NO_EXIST;
	else
		return getTTL(entry, ttl);
}


Response Directory::charge(BaseEntry* entry, short& newTTL)
{
	std::lock_guard<std::recursive_mutex> lock(entry->accessLock);
	if (entry->inDirectory())
	{
		newTTL = entry->getTTL();
		return Response::SUCCESS;
	}
	else
		return Response::NO_EXIST;
}

Response Directory::charge(const SourcePair& sourcePair, BaseEntry* cmdEntry, short& newTTL)
{
	auto entry = _findEntry(sourcePair);
	if (entry == nullptr)
		return Response::NO_EXIST;

	if (entry == cmdEntry)
		return charge(entry, newTTL);
	else
	{
		std::lock_guard<std::recursive_mutex> lock(entry->accessLock);
		if (_havePrivilegeToCharge(entry, cmdEntry))
		{
			entry->lastChargT = posix_time::second_clock::universal_time();
			return charge(entry, newTTL);
		}
		else
			return Response::NO_PRIVILAGE;
	}
}


Response Directory::getUpdateTocken(const SourcePair& sourcePair, BaseEntry* cmdEntry, UpdateTocken& updateTocken)
{
	auto entry = _findEntry(sourcePair);
	if (entry == nullptr)
		return Response::NO_EXIST;

	Response response;
	if (entry == cmdEntry)
		return getUpdateTocken(entry, updateTocken);
	else 
	{
		entry->accessLock.lock();
		if (entry->inDirectory())
		{
			auto maxPrivilege = entry->maxPrivilege(cmdEntry);
			if (maxPrivilege >= entry->permission.change)
			{
				updateTocken.EvB = entry;
				updateTocken.maxPrivilege = maxPrivilege;
				return Response::SUCCESS;
			}
			else
				response = Response::NO_PRIVILAGE;
		}
		else
			response = Response::NO_EXIST;
	}

	entry->accessLock.unlock();
	return response;
}

Response Directory::getUpdateTocken(BaseEntry* entry, UpdateTocken& updateTocken)
{
	entry->accessLock.lock();
	if (entry->inDirectory())
	{
		updateTocken.EvB = entry;
		updateTocken.maxPrivilege = Privilege::RESTRICTED_ENTRY;
		return Response::SUCCESS;
	}
	else
	{
		entry->accessLock.unlock();
		return Response::NO_EXIST;
	}
}

void Directory::releaseUpdateTocken(UpdateTocken& updateTocken)
{
	updateTocken.EvB->accessLock.unlock();
}

Response Directory::update(UpdateTocken& updateTocken, const Permission& perm)
{
	if (updateTocken.maxPrivilege >= perm.change && 
		updateTocken.maxPrivilege >= perm.remove && 
		updateTocken.maxPrivilege >= perm.charge)
	{
		updateTocken.EvB->permission = perm;
		return Response::SUCCESS;
	}
	else
		return Response::NO_PRIVILAGE;
}

void Directory::update(UpdateTocken& updateTocken, const std::string_view& desc)
{
	updateTocken.EvB->description = desc;
}

Response Directory::searchEntry(const SourcePair& sourcePair, BaseEntry*& entry)
{
	auto searchingEntry = _findEntry(sourcePair);
	if (searchingEntry == nullptr)
		return Response::NO_EXIST;
	if (searchingEntry->inDirectory())
	{
		entry = searchingEntry;
		return Response::SUCCESS;
	}
	return::Response::NO_EXIST;
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


Response Directory::flushEntries(std::string& writeBuffer, std::size_t flushCount)
{
	if (BaseEntry::begin == nullptr)
		return Response::NO_EXIST;
	else
	{
		auto entryPtr = BaseEntry::begin;
		while (entryPtr != nullptr && flushCount != 0)
		{
			std::lock_guard<std::recursive_mutex> lock(entryPtr->accessLock);
			entryPtr->printExpand(writeBuffer);
			entryPtr = entryPtr->next;
			flushCount--;
			if (entryPtr == nullptr)
				writeBuffer += '\x1e';		//!< Record separator
			else
				writeBuffer += '\x1f';		//!< Unit separator
		}
		return Response::SUCCESS;
	}
}

void Directory::printBrief(BaseEntry* entry, std::string& writeBuffer)
{
	entry->printBrief(writeBuffer);
}

void Directory::printExpand(BaseEntry* entry, std::string& writeBuffer)
{
	std::lock_guard<std::recursive_mutex> lock(entry->accessLock);
	entry->printExpand(writeBuffer);
}

void Directory::printUID(const BaseEntry* entry, std::string& writeBuffer)
{
	writeBuffer += entry->UID;
}
