#include "sp_entry.h"
#include "cmd_interpreter.h"

const std::string BaseEntry::VER[] =
{
	"v4",
	"v6"
};
const char BaseEntry::PRI[] =
{
	'l',
	'p',
	'r'
};
int BaseEntry::entryCount = 0;
BaseEntry* BaseEntry::begin = nullptr;
BaseEntry* BaseEntry::end = nullptr;
std::mutex BaseEntry::entryTrainLock;

short BaseEntry::_tmAfterLastChrg()
{
	auto timeNow = posix_time::second_clock::universal_time();
	auto diffTime = timeNow - lastChargT;
	return (short)diffTime.minutes();
}

void BaseEntry::_doExpiryCheck()
{
	if (_tmAfterLastChrg() > (unsigned short)timeToLive)
		removeFromDirectory();
}


void BaseEntry::attachToPeer()
{
	timeToLive = TTL::RESTRICTED_TTL;
	iswithPeer = true;
}

void BaseEntry::detachFromPeer()
{
	iswithPeer = false;
	lastChargT = posix_time::second_clock::universal_time();
}

bool BaseEntry::inDirectory()
{
	if (!iswithPeer && isInDirectory)
		_doExpiryCheck();
	return isInDirectory;
}

Privilege BaseEntry::maxPrivilege(BaseEntry* cmdEntry)
{
	if (version == Version::V4)
		return ((EntryV4*)this)->maxPrivilege(cmdEntry);
	else
		return ((EntryV6*)this)->maxPrivilege(cmdEntry);
}

void BaseEntry::addToDirectory(TTL ttl)
{
	std::lock_guard<std::mutex> lock(entryTrainLock);
	isInDirectory = true;
	lastChargT = posix_time::second_clock::universal_time();
	timeToLive = ttl;
	entryCount++;

	if (begin == nullptr)
	{
		begin = this;
		end = this;
		this->next = nullptr;
		this->previous = nullptr;
	}
	else
	{
		end->next = this;
		this->previous = end;
		this->next = nullptr;
		end = this;
	}
}

void BaseEntry::removeFromDirectory()
{
	std::lock_guard<std::mutex> lock(entryTrainLock);
	if (isInDirectory)
	{
		isInDirectory = false;
		entryCount--;

		auto this_next = this->next;
		auto this_previous = this->previous;
		if (this_next == nullptr && this_previous == nullptr)
		{
			begin = nullptr;
			end = nullptr;
		}
		else if (this_next != nullptr && this_previous != nullptr)
		{
			this_previous->next = this_next;
			this_next->previous = this_previous;
		}
		else if (begin == this)
		{
			begin = this_next;
			begin->previous = nullptr;
		}
		else if (end == this)
		{
			this_previous->next = nullptr;
			end = this;
		}
	}
}

void BaseEntry::printBrief(std::string& strBuffer)
{
	if (version == Version::V4)
		strBuffer += VER[(short)Version::V4];
	else
		strBuffer += VER[(short)Version::V6];
	strBuffer += " " + ipAddress + " " + portNumber;
}

void BaseEntry::printExpand(std::string& strBuffer)
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

void BaseEntry::printUID(std::string& strBuffer)
{
	strBuffer += UID;
}

short BaseEntry::getTTL()
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

const Version& BaseEntry::getVersion()
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


BaseEntry* UpdateTocken::entry()
{
	return EvB;
}
