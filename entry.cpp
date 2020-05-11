#include "entry.h"
#include "cmd_interpreter.h"
#include <cppcodec/base64_rfc4648.hpp>

DLLController<Entry> Entry::dllController;

const std::string Entry::VER[] =
{
	"v4",
	"v6"
};

short Entry::_tmAfterLastChrg()
{
	auto timeNow = posix_time::second_clock::universal_time();
	auto diffTime = timeNow - lastChargT;
	return (short)diffTime.minutes();
}

void Entry::_doExpiryCheck()
{
	if (_tmAfterLastChrg() > (unsigned short)timeToLive)
		removeFromDirectory();
}

Entry::Entry(const SourcePairV4& sourcePair, asio::ip::address_v4 ipAddr, unsigned short portNum)
{
	spAddress.SPA.V4 = sourcePair;
	UID = cppcodec::base64_rfc4648::encode(spAddress.SPA.V4);
	spAddress.version = Version::V4;

	portNumber = std::to_string(portNum);
	ipAddress = ipAddr.to_string();
	createdT = posix_time::second_clock::universal_time();
	description = "[]";
}

Entry::Entry(const SourcePairV6& sourcePair, asio::ip::address_v6 ipAddr, unsigned short portNum)
{
	spAddress.SPA.V6 = sourcePair;
	UID = cppcodec::base64_rfc4648::encode(spAddress.SPA.V6);
	spAddress.version = Version::V6;

	portNumber = std::to_string(portNum);
	ipAddress = ipAddr.to_string();
	createdT = posix_time::second_clock::universal_time();
	description = "[]";
}

short Entry::getTTL()
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

void Entry::printExpand(std::string& writeBuffer)
{
	if (version() == Version::V4)
		writeBuffer += VER[(short)Version::V4];
	else
		writeBuffer += VER[(short)Version::V6];
	writeBuffer += " " + UID + " " + ipAddress + " " + portNumber + " ";
	writeBuffer += CmdInterpreter::toPermission(permission);
	writeBuffer += " " + description;
}

bool Entry::inDirectory()
{
	if (!iswithPeer && isInDirectory)
		_doExpiryCheck();
	return isInDirectory;
}

void Entry::addToDirectory(TTL ttl)
{
	isInDirectory = true;
	charge(ttl);
	dllNode.addToDLL(this);
}

void Entry::removeFromDirectory()
{
	timeToLive = TTL::LIBERAL_TTL;
	if (isInDirectory)
	{
		isInDirectory = false;
		dllNode.removeFromDLL(this);
	}
}

void Entry::assignDefualtPermission(const Privilege& maxPrivilege)
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

void Entry::printBrief(std::string& writeBuffer)
{
	if (version() == Version::V4)
		writeBuffer += VER[(short)Version::V4];
	else
		writeBuffer += VER[(short)Version::V6];
	writeBuffer += " " + ipAddress + " " + portNumber;
}

short Entry::charge(const TTL& newTTL)
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

void Entry::lock()
{
	accessLock.lock();
}

void Entry::unlock()
{
	accessLock.unlock();
}

const std::string& Entry::uid()
{
	return UID;
}

Privilege Entry::maxPrivilege(Entry* cmdEntry)
{
	if (version() != cmdEntry->version())
		return Privilege::LIBERAL_ENTRY;

	if (version() == Version::V4)
	{
		auto ipSize = spAddress.SPA.IPV4.size();
		if (spAddress.SPA.IPV4 == cmdEntry->spAddress.SPA.IPV4)
			if (portNumber == cmdEntry->portNumber)
				return Privilege::RESTRICTED_ENTRY;
			else
				return Privilege::PROTECTED_ENTRY;
		else
			return Privilege::LIBERAL_ENTRY;
	}
	else
	{
		auto ipSize = spAddress.SPA.IPV6.size();
		if (spAddress.SPA.IPV6 == cmdEntry->spAddress.SPA.IPV6)
			if (portNumber == cmdEntry->portNumber)
				return Privilege::RESTRICTED_ENTRY;
			else
				return Privilege::PROTECTED_ENTRY;
		else
			return Privilege::LIBERAL_ENTRY;
	}
}

bool Entry::canChargeWith(const Privilege& maxPrivilege)
{
	if (maxPrivilege >= permission.charge)
		return true;
	else
		return false;
}

bool Entry::canChangeWith(const Privilege& maxPrivilege)
{
	if (maxPrivilege >= permission.change)
		return true;
	else
		return false;
}

bool Entry::canRemoveWith(const Privilege& maxPrivilege)
{
	if (maxPrivilege >= permission.remove)
		return true;
	else
		return false;
}

void Entry::attachToPeer()
{
	timeToLive = TTL::RESTRICTED_TTL;
	iswithPeer = true;
}

void Entry::detachFromPeer()
{
	iswithPeer = false;
	lastChargT = posix_time::second_clock::universal_time();
}

const Version& Entry::version()
{
	return spAddress.version;
}