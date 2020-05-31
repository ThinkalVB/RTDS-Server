#include "directory.h"
#include "cmd_interpreter.h"

V4EntryMap Directory::entryMapV4;
std::mutex Directory::v4InsLock;
V6EntryMap Directory::entryMapV6;
std::mutex Directory::v6InsLock;
std::atomic<std::size_t> Directory::_entryCount = 0;

const ResponseData Directory::_searchEntry(const SourcePairV4& targetSPA)
{
	auto entryItr = entryMapV4.find(targetSPA);
	if (entryItr == entryMapV4.end())
		return ResponseData(Response::NO_EXIST);

	auto entry = entryItr->second;
	return entry->getPolicy();
}

const ResponseData Directory::_searchEntry(const SourcePairV6& targetSPA)
{
	auto entryItr = entryMapV6.find(targetSPA);
	if (entryItr == entryMapV6.end())
		return ResponseData(Response::NO_EXIST);

	auto entry = entryItr->second;
	return entry->getPolicy();
}


const ResponseData Directory::_createV4Entry(const SPaddress& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	std::lock_guard<std::mutex> lock(v4InsLock);
	auto entryItr = entryMapV4.find(targetSPA.spAddressV4());
	if (entryItr == entryMapV4.end())
	{
		auto responsePair = Entry::makeEntry(targetSPA, cmdSPA, mutData);
		if (responsePair.first == Response::SUCCESS)
		{
			_entryCount++;
			auto newEntry = responsePair.second;
			entryMapV4.insert(std::pair<SourcePairV4, Entry*>(targetSPA.spAddressV4(), newEntry));
		}
		return responsePair.first;
	}
	else
	{
		auto entry = entryItr->second;
		auto response = entry->reAddWith(cmdSPA, mutData);
		if (response == Response::SUCCESS)
			_entryCount++;
		return response;
	}
}

const ResponseData Directory::_createV6Entry(const SPaddress& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	std::lock_guard<std::mutex> lock(v6InsLock);
	auto entryItr = entryMapV6.find(targetSPA.spAddressV6());
	if (entryItr == entryMapV6.end())
	{
		auto responsePair = Entry::makeEntry(targetSPA, cmdSPA, mutData);
		if (responsePair.first == Response::SUCCESS)
		{
			_entryCount++;
			auto newEntry = responsePair.second;
			entryMapV6.insert(std::pair<SourcePairV6, Entry*>(targetSPA.spAddressV6(), newEntry));
		}
		return responsePair.first;
	}
	else
	{
		auto entry = entryItr->second;
		auto response = entry->reAddWith(cmdSPA, mutData);
		if (response == Response::SUCCESS)
			_entryCount++;
		return response;
	}
}


const ResponseData Directory::_removeEntry(const SourcePairV4& targetSPA, const SPaddress& cmdSPA)
{
	auto entryItr = entryMapV4.find(targetSPA);
	if (entryItr == entryMapV4.end())
		return ResponseData(Response::NO_EXIST);

	auto entry = entryItr->second;
	auto response =  entry->removeWith(cmdSPA);
	if (response == Response::SUCCESS)
		_entryCount--;
	return response;
}

const ResponseData Directory::_removeEntry(const SourcePairV6& targetSPA, const SPaddress& cmdSPA)
{
	auto entryItr = entryMapV6.find(targetSPA);
	if (entryItr == entryMapV6.end())
		return ResponseData(Response::NO_EXIST);

	auto entry = entryItr->second;
	auto response = entry->removeWith(cmdSPA);
	if (response == Response::SUCCESS)
		_entryCount--;
	return response;
}


const ResponseData Directory::_updateEntry(const SourcePairV4& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	auto entryItr = entryMapV4.find(targetSPA);
	if (entryItr == entryMapV4.end())
		return ResponseData(Response::NO_EXIST);

	auto entry = entryItr->second;
	return entry->updateWith(cmdSPA, mutData);
}

const ResponseData Directory::_updateEntry(const SourcePairV6& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	auto entryItr = entryMapV6.find(targetSPA);
	if (entryItr == entryMapV6.end())
		return ResponseData(Response::NO_EXIST);

	auto entry = entryItr->second;
	return entry->updateWith(cmdSPA, mutData);
}


const ResponseData Directory::_ttlEntry(const SourcePairV4& targetSPA)
{
	auto entryItr = entryMapV4.find(targetSPA);
	if (entryItr == entryMapV4.end())
		return ResponseData(Response::NO_EXIST);

	auto entry = entryItr->second;
	return entry->getTTL();
}

const ResponseData Directory::_ttlEntry(const SourcePairV6& targetSPA)
{
	auto entryItr = entryMapV6.find(targetSPA);
	if (entryItr == entryMapV6.end())
		return ResponseData(Response::NO_EXIST);

	auto entry = entryItr->second;
	return entry->getTTL();
}


const ResponseData Directory::_chargeEntry(const SourcePairV4& targetSPA, const SPaddress& cmdSPA)
{
	auto entryItr = entryMapV4.find(targetSPA);
	if (entryItr == entryMapV4.end())
		return ResponseData(Response::NO_EXIST);

	auto entry = entryItr->second;
	return entry->chargeWith(cmdSPA);
}

const ResponseData Directory::_chargeEntry(const SourcePairV6& targetSPA, const SPaddress& cmdSPA)
{
	auto entryItr = entryMapV6.find(targetSPA);
	if (entryItr == entryMapV6.end())
		return ResponseData(Response::NO_EXIST);

	auto entry = entryItr->second;
	return entry->chargeWith(cmdSPA);
}


const ResponseData Directory::createEntry(const SPaddress& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	if (targetSPA.version() == Version::V4)
		return _createV4Entry(targetSPA, cmdSPA, mutData);
	else
		return _createV6Entry(targetSPA, cmdSPA, mutData);
}

const ResponseData Directory::removeEntry(const SPaddress& targetSPA, const SPaddress& cmdSPA)
{
	if (targetSPA.version() == Version::V4)
		return _removeEntry(targetSPA.spAddressV4(), cmdSPA);
	else
		return _removeEntry(targetSPA.spAddressV6(), cmdSPA);
}

const ResponseData Directory::updateEntry(const SPaddress& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	if (targetSPA.version() == Version::V4)
		return _updateEntry(targetSPA.spAddressV4(), cmdSPA, mutData);
	else
		return _updateEntry(targetSPA.spAddressV6(), cmdSPA, mutData);
}

const ResponseData Directory::ttlEntry(const SPaddress& targetSPA)
{
	if (targetSPA.version() == Version::V4)
		return _ttlEntry(targetSPA.spAddressV4());
	else
		return _ttlEntry(targetSPA.spAddressV6());
}

const ResponseData Directory::chargeEntry(const SPaddress& targetSPA, const SPaddress& cmdSPA)
{
	if (targetSPA.version() == Version::V4)
		return _chargeEntry(targetSPA.spAddressV4(), cmdSPA);
	else
		return _chargeEntry(targetSPA.spAddressV6(), cmdSPA);
}

const ResponseData Directory::searchEntry(const SPaddress& targetSPA)
{
	if (targetSPA.version() == Version::V4)
		return _searchEntry(targetSPA.spAddressV4());
	else
		return _searchEntry(targetSPA.spAddressV6());
}

void Directory::printEntryWith(std::string& writeBuffer, const MutableData& policyMD)
{
	auto printedEntryCount = 0;
	auto entryItrV4 = entryMapV4.begin();
	while (entryItrV4 != entryMapV4.end())
	{
		auto entry = entryItrV4->second;
		if (entry->printIfComparable(writeBuffer, policyMD))
			printedEntryCount++;
		entryItrV4++;
	}

	auto entryItrV6 = entryMapV6.begin();
	while (entryItrV6 != entryMapV6.end())
	{
		auto entry = entryItrV6->second;
		if (entry->printIfComparable(writeBuffer, policyMD))
			printedEntryCount++;
		entryItrV6++;
	}

	if (printedEntryCount == 0)
		writeBuffer += CmdInterpreter::RESP[(short)Response::NO_EXIST];
}

std::size_t Directory::entryCount()
{
	return _entryCount;
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
