#include "entry.h"
#include "cmd_interpreter.h"

std::queue<Entry*> Entry::entryRecycler;
std::mutex Entry::recycleLock;

unsigned short Entry::_tmAfterLastChrg() const
{
	auto timeNow = posix_time::second_clock::universal_time();
	auto diffTime = timeNow - _lastChargT;
	return (unsigned short)diffTime.minutes();
}

void Entry::recycleEntry(Entry* entryPtr)
{
	std::lock_guard<std::mutex> lock(recycleLock);
	entryRecycler.push(entryPtr);
}


Entry::Entry(const SPaddress& spAddr) : _spAddress(spAddr)
{
	_uid = _spAddress.toUID();
	_portNumber = _spAddress.portNumber();
	_ipAddress = _spAddress.ipAddress();
	_createdT = posix_time::second_clock::universal_time();
}

Entry::Entry(const SPaddress& spAddr, const MutableData& mutData, const Privilege maxPriv) : Entry(spAddr)
{
	if (mutData.haveDescription())
		_description = mutData.description();
	else
		_description = "[]";

	if (mutData.havePermission())
		_permission = mutData.permission();
	else
		_permission = CmdInterpreter::toDefPermission(maxPriv);
	_timeToLive = CmdInterpreter::toInitialTTL(maxPriv);
}


bool Entry::expired() const
{
	if (_tmAfterLastChrg() > _timeToLive)
		return true;
	else
		return false;
}

short Entry::getTTL() const
{
	if (_iswithPeer)
		return (short)_timeToLive;
	else
	{
		short ttl = (short)_timeToLive - _tmAfterLastChrg();
		if (ttl > 0)
			return ttl;
		else
			return 0;
	}
}

const std::string& Entry::uid() const
{
	return _uid;
}

void Entry::printExpand(std::string& writeBuffer) const
{
	if (_spAddress.version() == Version::V4)
		writeBuffer += STR_V4;
	else
		writeBuffer += STR_V6;
	writeBuffer += " " + _uid + " " + _ipAddress + " " + _portNumber + " ";
	writeBuffer += CmdInterpreter::toPermission(_permission);
	writeBuffer += " " + _description;
}

short Entry::charge()
{
	auto timePassed = _tmAfterLastChrg();
	_lastChargT = posix_time::second_clock::universal_time();
	_timeToLive += timePassed;

	if (_timeToLive > MAX_TTL)
		_timeToLive = MAX_TTL;
	return _timeToLive;
}

bool Entry::haveSamePolicy(const Policy& policy)
{
	if (_description == policy.description() &&
		_permission == policy.permission())
		return true;
	else
		return false;
}

bool Entry::canChargeWith(const SPaddress& spAddr) const
{
	auto maxPrivilege = _spAddress.maxPrivilege(spAddr);
	if (maxPrivilege >= _permission.charge)
		return true;
	else
		return false;
}

bool Entry::canRemoveWith(const SPaddress& spAddr) const
{
	auto maxPrivilege = _spAddress.maxPrivilege(spAddr);
	if (maxPrivilege >= _permission.remove)
		return true;
	else
		return false;
}

bool Entry::tryUpdateEntry(const SPaddress& spAddr, const MutableData& data)
{
	auto maxPriv = _spAddress.maxPrivilege(spAddr);
	if (maxPriv >= _permission.change)
	{
		if (data.havePermission())
		{
			if (CmdInterpreter::isValid(data.permission(), maxPriv))
				_permission = data.permission();
			else
				return false;
		}

		if (data.haveDescription())
			_description = data.description();
		return true;
	}
	else
		return false;
}