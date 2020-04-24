#include "directory.h"
#include "cmd_interpreter.h"

std::map<sourcePairV4, EntryV4*> Directory::V4EntryMap;
std::map<sourcePairV6, EntryV6*> Directory::V6EntryMap;

std::mutex Directory::V4insertionLock;
std::mutex Directory::V6insertionLock;
int Directory::entryCount = 0;

__base_entry* Directory::_findEntry(const sourcePairV4& sourcePair)
{
	auto entry = V4EntryMap.find(sourcePair);
	if (entry != V4EntryMap.end())
		return entry->second;
	else 
		return nullptr;
}

__base_entry* Directory::_findEntry(const sourcePairV6& sourcePair)
{
	auto entry = V6EntryMap.find(sourcePair);
	if (entry != V6EntryMap.end())
		return entry->second;
	else
		return nullptr;
}

__base_entry* Directory::_findEntry(const SourcePair& sourcePair)
{
	if (sourcePair.version == Version::V4)
		return _findEntry(sourcePair.SP.V4);
	else
		return _findEntry(sourcePair.SP.V6);
}


bool Directory::_havePrivilegeToCharge(__base_entry* entry, __base_entry* cmdEntry)
{
	auto maxPrivilege = entry->maxPrivilege(cmdEntry);
	if (maxPrivilege >= entry->permission.charge)
		return true;
	else
		return false;
}

bool Directory::_havePrivilegeToRemove(__base_entry* entry, __base_entry* cmdEntry)
{
	auto maxPrivilege = entry->maxPrivilege(cmdEntry);
	if (maxPrivilege >= entry->permission.remove)
		return true;
	else
		return false;
}

bool Directory::_havePrivilegeToChange(__base_entry* entry, __base_entry* cmdEntry)
{
	auto maxPrivilege = entry->maxPrivilege(cmdEntry);
	if (maxPrivilege >= entry->permission.change)
		return true;
	else
		return false;
}


__base_entry* Directory::makeEntry(asio::ip::address_v4 ipAdd, unsigned short portNum)
{
	sourcePairV4 sourcePair;
	CmdInterpreter::makeSourcePair(ipAdd, portNum, sourcePair);

	std::lock_guard<std::mutex> lock(V4insertionLock);
	auto entryPtr = _findEntry(sourcePair);
	if (entryPtr == nullptr)
	{
		entryPtr = new EntryV4(sourcePair, ipAdd, portNum);
		V4EntryMap.insert(std::pair<sourcePairV4, EntryV4*>(sourcePair, (EntryV4*)entryPtr));
	}
	return entryPtr;
}

__base_entry* Directory::makeEntry(asio::ip::address_v6 ipAdd, unsigned short portNum)
{
	sourcePairV6 sourcePair;
	CmdInterpreter::makeSourcePair(ipAdd, portNum, sourcePair);

	std::lock_guard<std::mutex> lock(V6insertionLock);
	auto entryPtr = _findEntry(sourcePair);
	if (entryPtr == nullptr)
	{
		entryPtr = new EntryV6(sourcePair, ipAdd, portNum);
		V6EntryMap.insert(std::pair<sourcePairV6, EntryV6*>(sourcePair, (EntryV6*)entryPtr));
	}
	return entryPtr;
}


Response Directory::addToDir(__base_entry* entry)
{
	std::lock_guard<std::recursive_mutex> lock(entry->accessLock);
	if (entry->inDirectory())
		return Response::REDUDANT_DATA;
	else
	{
		entry->isInDirectory = true;
		entryCount++;
		return Response::SUCCESS;
	}
}


Response Directory::removeFromDir(__base_entry* entry)
{
	std::lock_guard<std::recursive_mutex> lock(entry->accessLock);
	if (entry->inDirectory())
	{
		entry->isInDirectory = false;
		entryCount--;
		return Response::SUCCESS;
	}
	else
		return Response::NO_EXIST;
}

Response Directory::removeFromDir(const SourcePair& sourcePair, __base_entry* cmdEntry)
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
	

Response Directory::getTTL(__base_entry* entry, short& ttl)
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


Response Directory::charge(__base_entry* entry, short& newTTL)
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

Response Directory::charge(const SourcePair& sourcePair, __base_entry* cmdEntry, short& newTTL)
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

Response Directory::acquireUpdateLock(const SourcePair& sourcePair, __base_entry* cmdEntry, Entry& entryStruct)
{
	auto entry = _findEntry(sourcePair);
	if (entry == nullptr)
		return Response::NO_EXIST;

	entry->accessLock.lock();
	Response response;
	if (entry == cmdEntry)
	{
		if (entry->inDirectory())
		{
			entryStruct.Ev = entry;
			response = Response::SUCCESS;
		}
		else
			response = Response::NO_EXIST;
	}
	else 
	{
		if (entry->inDirectory())
		{
			if (_havePrivilegeToChange(entry, cmdEntry))
			{
				entryStruct.Ev = entry;
				response = Response::SUCCESS;
			}
			else
				response = Response::NO_PRIVILAGE;
		}
		else
			response = Response::NO_EXIST;
	}

	if (response != Response::SUCCESS)
		entry->accessLock.unlock();
	return response;
}

void Directory::releaseLock(__base_entry* entry)
{
	entry->accessLock.unlock();
}

void Directory::update(__base_entry* entry, const Permission& perm)
{
	entry->permission = perm;
}

void Directory::update(__base_entry* entry, const std::string_view& desc)
{
	entry->description = desc;
}


int Directory::getEntryCount()
{
	std::lock_guard<std::mutex> lock1(V4insertionLock);
	std::lock_guard<std::mutex> lock2(V6insertionLock);
	return entryCount;
}