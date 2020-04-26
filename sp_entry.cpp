#include "sp_entry.h"
#include "cmd_interpreter.h"

const std::string __base_entry::VER[] =
{
	"v4",
	"v6"
};
const char __base_entry::PRI[] =
{
	'l',
	'p',
	'r'
};
int __base_entry::entryCount = 0;
__base_entry* __base_entry::beginPtr = nullptr;
__base_entry* __base_entry::endPtr = nullptr;

short __base_entry::_tmAfterLastChrg()
{
	auto timeNow = posix_time::second_clock::universal_time();
	auto diffTime = timeNow - lastChargT;
	return (short)diffTime.minutes();
}

void __base_entry::_doExpiryCheck()
{
	if (_tmAfterLastChrg() > (unsigned short)timeToLive)
		isInDirectory = false;
}


void __base_entry::attachToPeer()
{
	timeToLive = TTL::RESTRICTED_TTL;
	iswithPeer = true;
}

void __base_entry::detachFromPeer()
{
	iswithPeer = false;
	lastChargT = posix_time::second_clock::universal_time();
}

bool __base_entry::inDirectory()
{
	if (!iswithPeer && isInDirectory)
		_doExpiryCheck();
	return isInDirectory;
}

Privilege __base_entry::maxPrivilege(__base_entry* cmdEntry)
{
	if (version == Version::V4)
		return ((EntryV4*)this)->maxPrivilege(cmdEntry);
	else
		return ((EntryV6*)this)->maxPrivilege(cmdEntry);
}

void __base_entry::addToDirectory()
{
	isInDirectory = true;
	lastChargT = posix_time::second_clock::universal_time();
	entryCount++;
}

void __base_entry::removeFromDirectory()
{
	isInDirectory = false;
	entryCount--;
}

void __base_entry::printBrief(std::string& strBuffer)
{
	if (version == Version::V4)
		strBuffer += VER[(short)Version::V4];
	else
		strBuffer += VER[(short)Version::V6];
	strBuffer += " " + ipAddress + " " + portNumber;
}

void __base_entry::printExpand(std::string& strBuffer)
{
	if (version == Version::V4)
		strBuffer += VER[(short)Version::V4];
	else
		strBuffer += VER[(short)Version::V6];
	strBuffer += " " + UID + " " + ipAddress + " " + portNumber + " ";
	strBuffer += PRI[(short)permission.change];
	strBuffer += PRI[(short)permission.charge];
	strBuffer += PRI[(short)permission.remove];
	strBuffer += " " + description;
}

void __base_entry::printUID(std::string& strBuffer)
{
	strBuffer += UID;
}

short __base_entry::getTTL()
{
	if (iswithPeer)
		return (short)timeToLive;
	else
	{
		short ttl = (short)timeToLive - _tmAfterLastChrg();
		if (ttl > 0)
			return ttl;
		else
			return 0;
	}
}

const Version& __base_entry::getVersion()
{
	return version;
}


const size_t& CommandElement::size()
{
	return _size;
}

void CommandElement::reset_for_read()
{
	_index = 0;
}

void CommandElement::reset()
{
	_size = 0;
	_index = 0;
}

const std::string_view& CommandElement::pop_front()
{
	_size--;
	auto currIndex = _index;
	_index++;
	return element[currIndex];
}

void CommandElement::pop_front(int count)
{
	_size = _size - count;
	_index = _index + count;
}

const std::string_view& CommandElement::peek()
{
	return element[_index];
}

const std::string_view& CommandElement::peek_next()
{
	return element[_index + 1];
}

void CommandElement::push_back(std::string_view elem)
{
	_size++;
	element[_index] = elem;
	_index++;
}


unsigned short SourcePair::portNumber()
{
	unsigned short portNumber;
	if (version == Version::V4)
		memcpy(&portNumber, SP.V4.data() + SP.IPV4.size(), 2);
	else
		memcpy(&portNumber, SP.V6.data() + SP.IPV6.size(), 2);

	#ifdef BOOST_ENDIAN_LITTLE_BYTE
	CmdInterpreter::byteSwap(portNumber);
	#endif
	return portNumber;
}


__base_entry* UpdateTocken::entry()
{
	return EvB;
}
