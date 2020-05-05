#include "sp_entry.h"
#include "cmd_interpreter.h"

const std::string BaseEntry::VER[] =
{
	"v4",
	"v6"
};

DLLController<BaseEntry> BaseEntry::dllController;

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
	if (this == cmdEntry)
		return Privilege::RESTRICTED_ENTRY;
	else if (version == Version::V4)
		return ((EntryV4*)this)->maxPrivilege(cmdEntry);
	else
		return ((EntryV6*)this)->maxPrivilege(cmdEntry);
}

void BaseEntry::assignDefualtPermission(Privilege maxPrivilege)
{
	if (maxPrivilege == Privilege::RESTRICTED_ENTRY || maxPrivilege == Privilege::PROTECTED_ENTRY)
	{
		permission.charge = Privilege::PROTECTED_ENTRY;
		permission.change = Privilege::PROTECTED_ENTRY;
		permission.remove = Privilege::PROTECTED_ENTRY;
	}
	else
	{
		permission.charge = Privilege::LIBERAL_ENTRY;
		permission.change = Privilege::LIBERAL_ENTRY;
		permission.remove = Privilege::LIBERAL_ENTRY;
	}
}


void BaseEntry::lock()
{
	accessLock.lock();
}

void BaseEntry::unlock()
{
	accessLock.unlock();
}

const std::string& BaseEntry::uid()
{
	return UID;
}


void BaseEntry::addToDirectory(TTL ttl)
{
	isInDirectory = true;
	charge(ttl);
	dllNode.addToDLL(this);
}

void BaseEntry::removeFromDirectory()
{
	timeToLive = TTL::LIBERAL_TTL;
	if (isInDirectory)
	{
		isInDirectory = false;
		dllNode.removeFromDLL(this);
	}
}

short BaseEntry::charge(const TTL& newTTL)
{
	auto timeRemaining = getTTL();
	if (timeRemaining < (short)newTTL)
	{
		lastChargT = posix_time::second_clock::universal_time();
		timeToLive = newTTL;
		return (short)newTTL;
	}
	else
		return timeRemaining;
}

void BaseEntry::printBrief(std::string& writeBuffer)
{
	if (version == Version::V4)
		writeBuffer += VER[(short)Version::V4];
	else
		writeBuffer += VER[(short)Version::V6];
	writeBuffer += " " + ipAddress + " " + portNumber;
}

void BaseEntry::printExpand(std::string& writeBuffer)
{
	if (version == Version::V4)
		writeBuffer += VER[(short)Version::V4];
	else
		writeBuffer += VER[(short)Version::V6];
	writeBuffer += " " + UID + " " + ipAddress + " " + portNumber + " ";
	writeBuffer += CmdInterpreter::toPermission(permission);
	writeBuffer += " " + description;
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


bool BaseEntry::canChargeWith(const Privilege& maxPrivilege)
{
	if (maxPrivilege >= permission.charge)
		return true;
	else
		return false;
}

bool BaseEntry::canChangeWith(const Privilege& maxPrivilege)
{
	if (maxPrivilege >= permission.change)
		return true;
	else
		return false;
}

bool BaseEntry::canRemoveWith(const Privilege& maxPrivilege)
{
	if (maxPrivilege >= permission.remove)
		return true;
	else
		return false;
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
