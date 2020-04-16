#include "sp_entry.h"

const std::string EntryV4::versionID = "v4";
const std::string EntryV6::versionID = "v6";

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

bool entryBase::haveExpired(short& expireIn)
{
	if (iswithPeer)
	{
		expireIn = (short)TTL::RESTRICTED_TTL;
		return false;
	}
	auto timeNow = posix_time::second_clock::local_time();
	auto diffTime = timeNow - addedTime;
	if (diffTime.minutes() > (short)timeToLive)
		return true;
	else
	{
		expireIn = (short)diffTime.minutes();
		return false;
	}
}

bool entryBase::haveExpired()
{
	if (iswithPeer)
		return false;
	auto timeNow = posix_time::second_clock::local_time();
	auto diffTime = timeNow - addedTime;
	if (diffTime.minutes() > (short)timeToLive)
		return true;
	else
		return false;
}

bool entryBase::inDirectory()
{
	return isInDirectory;
}

void entryBase::_chargeEntry()
{
	std::lock_guard<std::mutex> lock(accessLock);
	addedTime = posix_time::second_clock::local_time();
}
