#include "sp_entry.h"

const std::string EntryV4::versionID = "v4";
const std::string EntryV6::versionID = "v6";

void entryBase::attachToPeer()
{
	timeToLive = TTL::CONNECTED_TTL;
	iswithPeer = true;
}

void entryBase::detachFromPeer()
{
	timeToLive = TTL::RESTRICTED_TTL;
	iswithPeer = false;
}

bool entryBase::haveExpired(short& expireIn)
{
	if (timeToLive == TTL::CONNECTED_TTL)
		return false;
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
	if (timeToLive == TTL::CONNECTED_TTL)
		return false;
	auto timeNow = posix_time::second_clock::local_time();
	auto diffTime = timeNow - addedTime;
	if (diffTime.minutes() > (short)timeToLive)
		return true;
	else
		return false;
}

void entryBase::chargeEntry()
{
	addedTime = posix_time::second_clock::local_time();
}
