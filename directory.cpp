#include "directory.h"
#include "cmd_interpreter.h"

V4EntryMap Directory::entryMapV4;
V6EntryMap Directory::entryMapV6;

std::mutex Directory::v4MapAccess;
std::mutex Directory::v6MapAccess;

ResponsePair Directory::_searchEntry(const SourcePairV4& targetSPA)
{
	std::lock_guard<std::mutex> lock(v4MapAccess);
	auto entryItr = entryMapV4.find(targetSPA);
	if (entryItr == entryMapV4.end())
		return std::make_pair(Response::NO_EXIST, nullptr);
	else
	{
		auto entry = entryItr->second;
		if (entry->expired())
		{
			entryMapV4.erase(entryItr);
			Entry::recycleEntry(entry);
			return std::make_pair(Response::NO_EXIST, entry);
		}
		else
			return std::make_pair(Response::SUCCESS, entry);
	}
}

ResponsePair Directory::_searchEntry(const SourcePairV6& targetSPA)
{
	std::lock_guard<std::mutex> lock(v6MapAccess);
	auto entryItr = entryMapV6.find(targetSPA);
	if (entryItr == entryMapV6.end())
		return std::make_pair(Response::NO_EXIST, nullptr);
	else
	{
		auto entry = entryItr->second;
		if (entry->expired())
		{
			entryMapV6.erase(entryItr);
			Entry::recycleEntry(entry);
			return std::make_pair(Response::NO_EXIST, entry);
		}
		else
			return std::make_pair(Response::SUCCESS, entry);
	}
}

void Directory::_searchV4Entry(const Policy& policy, std::string& writeBuffer)
{
	std::lock_guard<std::mutex> lock(v4MapAccess);
	auto entryItr = entryMapV4.begin();
	while (entryItr != entryMapV4.end())
	{
		auto entry = entryItr->second;
		if (entry->expired())
		{
			entryMapV4.erase(entryItr);
			Entry::recycleEntry(entry);
		}
		else
		{
			if (entry->haveSamePolicy(policy))
			{
				entry->printExpand(writeBuffer);
				writeBuffer += '\x1f';		//!< Unit separator
			}
		}
		entryItr++;
	}
}

void Directory::_searchV6Entry(const Policy& policy, std::string& writeBuffer)
{
	std::lock_guard<std::mutex> lock(v6MapAccess);
	auto entryItr = entryMapV6.begin();
	while (entryItr != entryMapV6.end())
	{
		auto entry = entryItr->second;
		if (entry->expired())
		{
			entryMapV6.erase(entryItr);
			Entry::recycleEntry(entry);
		}
		else
		{
			if (entry->haveSamePolicy(policy))
			{
				entry->printExpand(writeBuffer);
				writeBuffer += '\x1f';		//!< Unit separator
			}
		}
		entryItr++;
	}
}


ResponsePair Directory::_createV4Entry(const SPaddress& spAddr, const MutableData& mutData, const Privilege maxPriv)
{
	std::lock_guard<std::mutex> lock(v4MapAccess);
	auto entryItr = entryMapV4.find(spAddr.spAddressV4());
	if (entryItr == entryMapV4.end())
	{
		auto newEntry = new Entry(spAddr, mutData, maxPriv);
		entryMapV4.insert(std::pair<SourcePairV4, Entry*>(spAddr.spAddressV4(), newEntry));
		return std::make_pair(Response::SUCCESS, newEntry);
	}
	else
	{
		auto oldEntry = entryItr->second;
		if (oldEntry->expired())
		{
			Entry::recycleEntry(oldEntry);
			auto newEntry = new Entry(spAddr, mutData, maxPriv);
			entryItr->second = newEntry;
			return std::make_pair(Response::SUCCESS, newEntry);
		}
		else
			return std::make_pair(Response::REDUDANT_DATA, oldEntry);
	}
}

ResponsePair Directory::_createV6Entry(const SPaddress& spAddr, const MutableData& mutData, const Privilege maxPriv)
{
	std::lock_guard<std::mutex> lock(v6MapAccess);
	auto entryItr = entryMapV6.find(spAddr.spAddressV6());
	if (entryItr == entryMapV6.end())
	{
		auto newEntry = new Entry(spAddr, mutData, maxPriv);
		entryMapV6.insert(std::pair<SourcePairV6, Entry*>(spAddr.spAddressV6(), newEntry));
		return std::make_pair(Response::SUCCESS, newEntry);
	}
	else
	{
		auto oldEntry = entryItr->second;
		if (oldEntry->expired())
		{
			Entry::recycleEntry(oldEntry);
			auto newEntry = new Entry(spAddr, mutData, maxPriv);
			entryItr->second = newEntry;
			return std::make_pair(Response::SUCCESS, newEntry);
		}
		else
			return std::make_pair(Response::REDUDANT_DATA, oldEntry);
	}
}


ResponsePair Directory::_removeEntry(const SourcePairV4& targetSPA, const SPaddress& cmdSPA)
{
	std::lock_guard<std::mutex> lock(v4MapAccess);
	auto entryItr = entryMapV4.find(targetSPA);

	if (entryItr == entryMapV4.end())
		return std::make_pair(Response::NO_EXIST, nullptr);
	
	auto entry = entryItr->second;
	if (entry->expired())
	{
		entryMapV4.erase(entryItr);
		Entry::recycleEntry(entry);
		return std::make_pair(Response::NO_EXIST, entry);
	}
	else
	{
		if (entry->canRemoveWith(cmdSPA))
		{
			entryMapV4.erase(entryItr);
			Entry::recycleEntry(entry);
			return std::make_pair(Response::SUCCESS, entry);
		}
		else
			return std::make_pair(Response::NO_PRIVILAGE, entry);
	}
}

ResponsePair Directory::_removeEntry(const SourcePairV6& targetSPA, const SPaddress& cmdSPA)
{
	std::lock_guard<std::mutex> lock(v6MapAccess);
	auto entryItr = entryMapV6.find(targetSPA);

	if (entryItr == entryMapV6.end())
		return std::make_pair(Response::NO_EXIST, nullptr);

	auto entry = entryItr->second;
	if (entry->expired())
	{
		entryMapV6.erase(entryItr);
		Entry::recycleEntry(entry);
		return std::make_pair(Response::NO_EXIST, entry);
	}
	else
	{
		if (entry->canRemoveWith(cmdSPA))
		{
			entryMapV6.erase(entryItr);
			Entry::recycleEntry(entry);
			return std::make_pair(Response::SUCCESS, entry);
		}
		else
			return std::make_pair(Response::NO_PRIVILAGE, entry);
	}
}


ResponsePair Directory::_updateEntry(const SourcePairV4& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	std::lock_guard<std::mutex> lock(v4MapAccess);
	auto entryItr = entryMapV4.find(targetSPA);

	if (entryItr == entryMapV4.end())
		return std::make_pair(Response::NO_EXIST, nullptr);

	auto entry = entryItr->second;
	if (entry->expired())
	{
		entryMapV4.erase(entryItr);
		Entry::recycleEntry(entry);
		return std::make_pair(Response::NO_EXIST, entry);
	}
	else
	{
		if (entry->tryUpdateEntry(cmdSPA, mutData))
			return std::make_pair(Response::SUCCESS, entry);
		else
			return std::make_pair(Response::NO_PRIVILAGE, entry);
	}
}

ResponsePair Directory::_updateEntry(const SourcePairV6& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	std::lock_guard<std::mutex> lock(v6MapAccess);
	auto entryItr = entryMapV6.find(targetSPA);

	if (entryItr == entryMapV6.end())
		return std::make_pair(Response::NO_EXIST, nullptr);

	auto entry = entryItr->second;
	if (entry->expired())
	{
		entryMapV6.erase(entryItr);
		Entry::recycleEntry(entry);
		return std::make_pair(Response::NO_EXIST, entry);
	}
	else
	{
		if (entry->tryUpdateEntry(cmdSPA, mutData))
			return std::make_pair(Response::SUCCESS, entry);
		else
			return std::make_pair(Response::NO_PRIVILAGE, entry);
	}
}


ResponseTTL Directory::_ttlEntry(const SourcePairV4& targetSPA)
{
	std::lock_guard<std::mutex> lock(v4MapAccess);
	auto entryItr = entryMapV4.find(targetSPA);

	if (entryItr == entryMapV4.end())
		return std::make_pair(Response::NO_EXIST, 0);

	auto entry = entryItr->second;
	if (entry->expired())
	{
		entryMapV4.erase(entryItr);
		Entry::recycleEntry(entry);
		return std::make_pair(Response::NO_EXIST, 0);
	}
	else
	{
		auto ttl = entry->getTTL();
		return std::make_pair(Response::SUCCESS, ttl);
	}
}

ResponseTTL Directory::_ttlEntry(const SourcePairV6& targetSPA)
{
	std::lock_guard<std::mutex> lock(v6MapAccess);
	auto entryItr = entryMapV6.find(targetSPA);

	if (entryItr == entryMapV6.end())
		return std::make_pair(Response::NO_EXIST, 0);

	auto entry = entryItr->second;
	if (entry->expired())
	{
		entryMapV6.erase(entryItr);
		Entry::recycleEntry(entry);
		return std::make_pair(Response::NO_EXIST, 0);
	}
	else
	{
		auto ttl = entry->getTTL();
		return std::make_pair(Response::SUCCESS, ttl);
	}
}


ResponseTTL Directory::_chargeEntry(const SourcePairV4& targetSPA, const SPaddress& cmdSPA)
{
	std::lock_guard<std::mutex> lock(v4MapAccess);
	auto entryItr = entryMapV4.find(targetSPA);

	if (entryItr == entryMapV4.end())
		return std::make_pair(Response::NO_EXIST, 0);

	auto entry = entryItr->second;
	if (entry->expired())
	{
		entryMapV4.erase(entryItr);
		Entry::recycleEntry(entry);
		return std::make_pair(Response::NO_EXIST, 0);
	}
	else
	{
		if (entry->canChargeWith(cmdSPA))
		{
			auto ttl = entry->charge();
			return std::make_pair(Response::SUCCESS, ttl);
		}
		else
			return std::make_pair(Response::NO_PRIVILAGE, 0);
	}
}

ResponseTTL Directory::_chargeEntry(const SourcePairV6& targetSPA, const SPaddress& cmdSPA)
{
	std::lock_guard<std::mutex> lock(v6MapAccess);
	auto entryItr = entryMapV6.find(targetSPA);

	if (entryItr == entryMapV6.end())
		return std::make_pair(Response::NO_EXIST, 0);

	auto entry = entryItr->second;
	if (entry->expired())
	{
		entryMapV6.erase(entryItr);
		Entry::recycleEntry(entry);
		return std::make_pair(Response::NO_EXIST, 0);
	}
	else
	{
		if (entry->canChargeWith(cmdSPA))
		{
			auto ttl = entry->charge();
			return std::make_pair(Response::SUCCESS, ttl);
		}
		else
			return std::make_pair(Response::NO_PRIVILAGE, 0);
	}
}


ResponsePair Directory::createEntry(const SPaddress& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	auto maxPriv = targetSPA.maxPrivilege(cmdSPA);
	if (mutData.havePermission())
		if (!CmdInterpreter::isValid(mutData.permission(), maxPriv))
			return std::make_pair(Response::NO_PRIVILAGE, nullptr);

	if (targetSPA.version() == Version::V4)
		return _createV4Entry(targetSPA, mutData, maxPriv);
	else
		return _createV6Entry(targetSPA, mutData, maxPriv);
}

ResponsePair Directory::removeEntry(const SPaddress& targetSPA, const SPaddress& cmdSPA)
{
	if (targetSPA.version() == Version::V4)
		return _removeEntry(targetSPA.spAddressV4(), cmdSPA);
	else
		return _removeEntry(targetSPA.spAddressV6(), cmdSPA);
}

ResponsePair Directory::updateEntry(const SPaddress& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	if (targetSPA.version() == Version::V4)
		return _updateEntry(targetSPA.spAddressV4(), cmdSPA, mutData);
	else
		return _updateEntry(targetSPA.spAddressV6(), cmdSPA, mutData);
}

ResponseTTL Directory::ttlEntry(const SPaddress& targetSPA)
{
	if (targetSPA.version() == Version::V4)
		return _ttlEntry(targetSPA.spAddressV4());
	else
		return _ttlEntry(targetSPA.spAddressV6());
}

ResponseTTL Directory::chargeEntry(const SPaddress& targetSPA, const SPaddress& cmdSPA)
{
	if (targetSPA.version() == Version::V4)
		return _chargeEntry(targetSPA.spAddressV4(), cmdSPA);
	else
		return _chargeEntry(targetSPA.spAddressV6(), cmdSPA);
}

ResponsePair Directory::searchEntry(const SPaddress& targetSPA)
{
	if (targetSPA.version() == Version::V4)
		return _searchEntry(targetSPA.spAddressV4());
	else
		return _searchEntry(targetSPA.spAddressV6());
}

void Directory::searchEntry(const Policy& policy, std::string& writeBuffer)
{
	_searchV4Entry(policy, writeBuffer);
	_searchV6Entry(policy, writeBuffer);
}


std::size_t Directory::entryCount()
{
	return (entryMapV4.size() + entryMapV6.size());
}

void Directory::clearDirectory()
{
	std::lock_guard<std::mutex> v4Lock(v4MapAccess);
	std::lock_guard<std::mutex> v6Lock(v6MapAccess);
	std::map<SourcePairV4, Entry*>::iterator itV4 = entryMapV4.begin();
	while (itV4 != entryMapV4.end())
	{
		delete itV4->second;
		itV4++;
	}
	entryMapV4.clear();

	std::map<SourcePairV6, Entry*>::iterator itV6 = entryMapV6.begin();
	while (itV6 != entryMapV6.end())
	{
		delete itV6->second;
		itV6++;
	}
	entryMapV6.clear();
}
