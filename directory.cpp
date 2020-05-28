#include "directory.h"
#include "cmd_interpreter.h"

V4EntryMap Directory::entryMapV4;
std::mutex Directory::v4InsRemMapLock;
V6EntryMap Directory::entryMapV6;
std::mutex Directory::v6InsRemMapLock;

ResponsePair Directory::_searchEntry(const SourcePairV4& targetSPA)
{
	/*
	auto entryItr = entryMapV4.find(targetSPA);
	if (entryItr == entryMapV4.end())
		return std::make_pair(Response::NO_EXIST, nullptr);

	auto entry = entryItr->second;
	if (entry->expired())
		return std::make_pair(Response::NO_EXIST, entry);
	else
		return std::make_pair(Response::SUCCESS, entry);
	*/
	return std::make_pair(Response::NO_EXIST, nullptr);
}

ResponsePair Directory::_searchEntry(const SourcePairV6& targetSPA)
{
	/*
	auto entryItr = entryMapV6.find(targetSPA);
	if (entryItr == entryMapV6.end())
		return std::make_pair(Response::NO_EXIST, nullptr);

	auto entry = entryItr->second;
	if (entry->expired())
		return std::make_pair(Response::NO_EXIST, entry);
	else
		return std::make_pair(Response::SUCCESS, entry);
	*/
	return std::make_pair(Response::NO_EXIST, nullptr);
}


ResponsePair Directory::_createV4Entry(const SPaddress& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	/*
	std::lock_guard<std::mutex> lock(v4InsRemMapLock);
	auto entryItr = entryMapV4.find(targetSPA.spAddressV4());
	if (entryItr == entryMapV4.end())
	{
		auto responsePair = Entry::makeEntry(targetSPA, cmdSPA, mutData);
		if (responsePair.first == Response::SUCCESS)
		{
			auto newEntry = responsePair.second;
			entryMapV4.insert(std::pair<SourcePairV4, Entry*>(targetSPA.spAddressV4(), newEntry));
		}
		return responsePair;
	}

	auto entry = entryItr->second;
	return std::make_pair(entry->tryAddEntry(cmdSPA, mutData), entry);
	*/
	return std::make_pair(Response::NO_EXIST, nullptr);
}

ResponsePair Directory::_createV6Entry(const SPaddress& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	/*
	std::lock_guard<std::mutex> lock(v6InsRemMapLock);
	auto entryItr = entryMapV6.find(targetSPA.spAddressV6());
	if (entryItr == entryMapV6.end())
	{
		auto responsePair = Entry::makeEntry(targetSPA, cmdSPA, mutData);
		if (responsePair.first == Response::SUCCESS)
		{
			auto newEntry = responsePair.second;
			entryMapV6.insert(std::pair<SourcePairV6, Entry*>(targetSPA.spAddressV6(), newEntry));
		}
		return responsePair;
	}

	auto entry = entryItr->second;
	return std::make_pair(entry->tryAddEntry(cmdSPA, mutData), entry);
	*/
	return std::make_pair(Response::NO_EXIST, nullptr);
}


ResponsePair Directory::_removeEntry(const SourcePairV4& targetSPA, const SPaddress& cmdSPA)
{
	/*
	auto entryItr = entryMapV4.find(targetSPA);
	if (entryItr == entryMapV4.end())
		return std::make_pair(Response::NO_EXIST, nullptr);

	auto entry = entryItr->second;
	return  std::make_pair(entry->tryRemoveWith(cmdSPA), entry);
	*/
	return std::make_pair(Response::NO_EXIST, nullptr);
}

ResponsePair Directory::_removeEntry(const SourcePairV6& targetSPA, const SPaddress& cmdSPA)
{
	/*
	auto entryItr = entryMapV6.find(targetSPA);
	if (entryItr == entryMapV6.end())
		return std::make_pair(Response::NO_EXIST, nullptr);

	auto entry = entryItr->second;
	return  std::make_pair(entry->tryRemoveWith(cmdSPA), entry);
	*/
	return std::make_pair(Response::NO_EXIST, nullptr);
}


ResponsePair Directory::_updateEntry(const SourcePairV4& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	/*
	auto entryItr = entryMapV4.find(targetSPA);
	if (entryItr == entryMapV4.end())
		return std::make_pair(Response::NO_EXIST, nullptr);

	auto entry = entryItr->second;
	return std::make_pair(entry->tryUpdateEntry(cmdSPA, mutData), entry);
	*/
	return std::make_pair(Response::NO_EXIST, nullptr);
}

ResponsePair Directory::_updateEntry(const SourcePairV6& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	/*
	auto entryItr = entryMapV6.find(targetSPA);
	if (entryItr == entryMapV6.end())
		return std::make_pair(Response::NO_EXIST, nullptr);

	auto entry = entryItr->second;
	return std::make_pair(entry->tryUpdateEntry(cmdSPA, mutData), entry);
	*/
	return std::make_pair(Response::NO_EXIST, nullptr);
}


ResponseTTL Directory::_ttlEntry(const SourcePairV4& targetSPA)
{
	/*
	auto entryItr = entryMapV4.find(targetSPA);
	if (entryItr == entryMapV4.end())
		return std::make_pair(Response::NO_EXIST, 0);

	auto entry = entryItr->second;
	return entry->tryGetTTL();
	*/
	return std::make_pair(Response::NO_EXIST, 0);
}

ResponseTTL Directory::_ttlEntry(const SourcePairV6& targetSPA)
{
	/*
	auto entryItr = entryMapV6.find(targetSPA);
	if (entryItr == entryMapV6.end())
		return std::make_pair(Response::NO_EXIST, 0);

	auto entry = entryItr->second;
	return entry->tryGetTTL();
	*/
	return std::make_pair(Response::NO_EXIST, 0);
}


ResponseTTL Directory::_chargeEntry(const SourcePairV4& targetSPA, const SPaddress& cmdSPA)
{
	/*
	auto entryItr = entryMapV4.find(targetSPA);
	if (entryItr == entryMapV4.end())
		return std::make_pair(Response::NO_EXIST, 0);

	auto entry = entryItr->second;
	return entry->tryChargeWith(cmdSPA);
	*/
	return std::make_pair(Response::NO_EXIST, 0);
}

ResponseTTL Directory::_chargeEntry(const SourcePairV6& targetSPA, const SPaddress& cmdSPA)
{
	/*
	auto entryItr = entryMapV6.find(targetSPA);
	if (entryItr == entryMapV6.end())
		return std::make_pair(Response::NO_EXIST, 0);

	auto entry = entryItr->second;
	return entry->tryChargeWith(cmdSPA);
	*/
	return std::make_pair(Response::NO_EXIST, 0);
}


ResponsePair Directory::createEntry(const SPaddress& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	/*
	if (targetSPA.version() == Version::V4)
		return _createV4Entry(targetSPA, cmdSPA, mutData);
	else
		return _createV6Entry(targetSPA, cmdSPA, mutData);
	*/
	return std::make_pair(Response::NO_EXIST, nullptr);
}

ResponsePair Directory::removeEntry(const SPaddress& targetSPA, const SPaddress& cmdSPA)
{
	/*
	if (targetSPA.version() == Version::V4)
		return _removeEntry(targetSPA.spAddressV4(), cmdSPA);
	else
		return _removeEntry(targetSPA.spAddressV6(), cmdSPA);
	*/
	return std::make_pair(Response::NO_EXIST, nullptr);
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


std::size_t Directory::entryCount()
{
	return (entryMapV4.size() + entryMapV6.size());
}

void Directory::clearDirectory()
{
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
