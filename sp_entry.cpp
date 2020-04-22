#include "sp_entry.h"

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

Version __base_entry::getVersion()
{
	return version;
}


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