#include "directory.h"
#include "cmd_interpreter.h"

V4EntryMap Directory::entryMapV4;
std::mutex Directory::v4InsRemMapLock;
V6EntryMap Directory::entryMapV6;
std::mutex Directory::v6InsRemMapLock;


ResponsePair Directory::_searchEntry(const SourcePairV4& targetSPA)
{
	return std::make_pair(Response::SUCCESS, nullptr);
}

ResponsePair Directory::_searchEntry(const SourcePairV6& targetSPA)
{
	return std::make_pair(Response::SUCCESS, nullptr);
}


ResponsePair Directory::_createV4Entry(const SPaddress& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	return std::make_pair(Response::SUCCESS, nullptr);
}

ResponsePair Directory::_createV6Entry(const SPaddress& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	return std::make_pair(Response::SUCCESS, nullptr);
}



ResponsePair Directory::_removeEntry(const SourcePairV4& targetSPA, const SPaddress& cmdSPA)
{
	return std::make_pair(Response::SUCCESS, nullptr);
}

ResponsePair Directory::_removeEntry(const SourcePairV6& targetSPA, const SPaddress& cmdSPA)
{
	return std::make_pair(Response::SUCCESS, nullptr);
}


ResponsePair Directory::_updateEntry(const SourcePairV4& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	return std::make_pair(Response::SUCCESS, nullptr);
}

ResponsePair Directory::_updateEntry(const SourcePairV6& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	return std::make_pair(Response::SUCCESS, nullptr);
}


ResponseTTL Directory::_ttlEntry(const SourcePairV4& targetSPA)
{
	return std::make_pair(Response::SUCCESS, 0);
}

ResponseTTL Directory::_ttlEntry(const SourcePairV6& targetSPA)
{
	return std::make_pair(Response::SUCCESS, 0);
}


ResponseTTL Directory::_chargeEntry(const SourcePairV4& targetSPA, const SPaddress& cmdSPA)
{
	return std::make_pair(Response::SUCCESS, 0);
}

ResponseTTL Directory::_chargeEntry(const SourcePairV6& targetSPA, const SPaddress& cmdSPA)
{
	return std::make_pair(Response::SUCCESS, 0);
}


ResponsePair Directory::createEntry(const SPaddress& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	if (targetSPA.version() == Version::V4)
		return _createV4Entry(targetSPA, cmdSPA, mutData);
	else
		return _createV6Entry(targetSPA, cmdSPA, mutData);
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
