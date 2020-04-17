#include "sp_entry.h"
#include "directory.h"

const std::string entryBase::VER[] =
{
	"v4",
	"v6"
};

const char entryBase::PRI[] =
{
	'l',
	'p',
	'r'
};

EntryV4::EntryV4()
{
	version = Version::V4;
	description = "[]";
}

EntryV6::EntryV6()
{
	version = Version::V6;
	description = "[]";
}

void entryBase::attachToPeer()
{
	timeToLive = TTL::RESTRICTED_TTL;
	iswithPeer = true;
}

void entryBase::detachFromPeer()
{
	iswithPeer = false;
	_chargeEntry();
}

void entryBase::printUID(std::string& strBuffer)
{
	strBuffer += UID;
}

void entryBase::printEntryCount(std::string& strBuffer)
{
	strBuffer += std::to_string(Directory::getEntryCount());
}

void entryBase::printBrief(std::string& strBuffer)
{
	if (version == Version::V4)
		strBuffer += VER[(short)Version::V4];
	else
		strBuffer += VER[(short)Version::V6];
	strBuffer += " " + ipAddress + " " + portNumber;
}

void entryBase::printTTL(std::string& strBuffer)
{
	std::lock_guard<std::mutex> lock(accessLock);
	if (iswithPeer)
		strBuffer += std::to_string((short)Privilege::RESTRICTED_ENTRY);
	else
	{
		auto timePassed = _getTimePassed();
		short ttl = -(timePassed - (short)timeToLive);
		if (ttl <= 0)
			strBuffer += "0";
		else
			strBuffer += std::to_string(ttl);
	}
}

void entryBase::printExpand(std::string& strBuffer)
{
	if (version == Version::V4)
		strBuffer += VER[(short)Version::V4];
	else
		strBuffer += VER[(short)Version::V6];
	strBuffer += " " + UID + " " + ipAddress + " " + portNumber + " ";
	std::lock_guard<std::mutex> lock(accessLock);
	strBuffer += PRI[(short)permission.change];
	strBuffer += PRI[(short)permission.charge];
	strBuffer += PRI[(short)permission.remove];
	strBuffer += " " + description;
}

bool entryBase::inDirectory()
{
	return isInDirectory;
}

void entryBase::_chargeEntry()
{
	std::lock_guard<std::mutex> lock(accessLock);
	lastChargT = posix_time::second_clock::local_time();
}

bool entryBase::_haveExpired()
{
	std::lock_guard<std::mutex> lock(accessLock);
	if (_getTimePassed() > (unsigned short)timeToLive)
		return true;
	else
		return false;
}

short entryBase::_getTimePassed()
{
	auto timeNow = posix_time::second_clock::local_time();
	auto diffTime = timeNow - lastChargT;
	return (short)diffTime.minutes();
}
