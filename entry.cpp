#include "entry.h"
#include "cmd_interpreter.h"

unsigned int Entry::_tmAfterLastChrg() const
{
	auto timeNow = posix_time::second_clock::universal_time();
	auto diffTime = timeNow - _lastChargT;
	return (unsigned short)diffTime.minutes();
}

void Entry::_charge()
{
	auto timePassed = _tmAfterLastChrg();
	_lastChargT = posix_time::second_clock::universal_time();
	_timeToLive += timePassed;

	if (_timeToLive > MAX_TTL)
		_timeToLive = MAX_TTL;
	_timeLeft = _timeToLive;
}

void Entry::_updateTimeLeft()
{
	if (!_expired)
	{
		auto timePassed = _tmAfterLastChrg();
		if (timePassed > _timeToLive)
		{
			_expired = true;
			_timeLeft = 0;
		}
		else
			_timeLeft = _timeToLive - timePassed;
	}
}

void Entry::_initialize(const MutableData& mutData, const Privilege& maxPriv)
{
	if (mutData.haveDescription())
		_policy.setDescription(mutData.description());
	else
		_policy.setDescription("[]");

	if (mutData.havePermission())
		_policy.setPermission(mutData.permission());
	else
		_policy.setPermission(CmdInterpreter::toDefPermission(maxPriv));

	_createdT = posix_time::second_clock::universal_time();
	_lastChargT = _createdT;
	_timeToLive = CmdInterpreter::toInitialTTL(maxPriv);
	_timeLeft = _timeToLive;
	_expired = false;
}

Entry::Entry(const SPaddress& spAddr, const MutableData& mutData, const Privilege maxPriv) : _spAddress(spAddr)
{
	_uid = _spAddress.toUID();
	_portNumber = _spAddress.portNumber();
	_ipAddress = _spAddress.ipAddress();
	_initialize(mutData, maxPriv);
}


const AdvResponse Entry::makeEntry(const SPaddress& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	auto maxPriv = targetSPA.maxPrivilege(cmdSPA);
	if (mutData.havePermission())
		if (!CmdInterpreter::isValid(mutData.permission(), maxPriv))
			return std::make_pair(ResponseData(Response::NO_PRIVILAGE), nullptr);

	auto entry = new Entry(targetSPA, mutData, maxPriv);
	return std::make_pair(ResponseData(Response::SUCCESS, entry->_policy, entry->_timeToLive), entry);
}

const ResponseData Entry::getTTL()
{
	std::lock_guard<std::mutex> lock(_entryLock);
	_updateTimeLeft();

	if (_expired)
		return ResponseData(Response::NO_EXIST);
	else
		return ResponseData(Response::SUCCESS, _timeLeft);
}

const ResponseData Entry::getPolicy()
{
	std::lock_guard<std::mutex> lock(_entryLock);
	_updateTimeLeft();

	if (_expired)
		return ResponseData(Response::NO_EXIST);
	else
		return ResponseData(Response::SUCCESS, _policy);
}

bool Entry::printIfComparable(std::string& writeBuffer, const MutableData& policyMD)
{
	std::lock_guard<std::mutex> lock(_entryLock);
	_updateTimeLeft();

	if (!_expired)
	{
		if (policyMD.description() == _policy.description() &&
			CmdInterpreter::isComparable(policyMD.permission(), _policy.permission()))
		{
			if (_spAddress.version() == Version::V4)
				writeBuffer += STR_V4;
			else
				writeBuffer += STR_V6;
			writeBuffer += " " + _ipAddress + " " + _portNumber;
			writeBuffer += '\x1f';		//!< Unit separator
			return true;
		}
	}
	return false;
}

const ResponseData Entry::chargeWith(const SPaddress& spAddr)
{
	auto maxPriv = _spAddress.maxPrivilege(spAddr);
	std::lock_guard<std::mutex> lock(_entryLock);
	_updateTimeLeft();

	if (!_expired)
	{
		if (_policy.canChargeWith(maxPriv))
		{
			_charge();
			return ResponseData(Response::SUCCESS, _timeLeft);
		}
		else
			return ResponseData(Response::NO_PRIVILAGE, _timeLeft);
	}
	else
		return ResponseData(Response::NO_EXIST);
}

const ResponseData Entry::removeWith(const SPaddress& spAddr)
{
	auto maxPriv = _spAddress.maxPrivilege(spAddr);
	std::lock_guard<std::mutex> lock(_entryLock);
	_updateTimeLeft();

	if (!_expired)
	{
		if (_policy.canRemoveWith(maxPriv))
		{
			_expired = true;
			return ResponseData(Response::SUCCESS, _policy);
		}
		else
			return ResponseData(Response::NO_PRIVILAGE);
	}
	else
		return ResponseData(Response::NO_EXIST);
}

const ResponseData Entry::updateWith(const SPaddress& spAddr, const MutableData& data)
{
	auto maxPriv = _spAddress.maxPrivilege(spAddr);
	std::lock_guard<std::mutex> lock(_entryLock);
	_updateTimeLeft();

	if (_expired)
		return ResponseData(Response::NO_EXIST);

	if (_policy.canUpdateWith(maxPriv))
	{
		if (data.havePermission())
		{
			if (CmdInterpreter::isValid(data.permission(), maxPriv))
				_policy.setPermission(data.permission());
			else
				return ResponseData(Response::NO_PRIVILAGE);
		}

		if (data.haveDescription())
			_policy.setDescription(data.description());
		return ResponseData(Response::SUCCESS, _policy);
	}
	else
		return ResponseData(Response::NO_PRIVILAGE);
}

const ResponseData Entry::reAddWith(const SPaddress& cmdSPA, const MutableData& mutData)
{
	auto maxPriv = _spAddress.maxPrivilege(cmdSPA);
	std::lock_guard<std::mutex> lock(_entryLock);
	_updateTimeLeft();

	if (!_expired)
		return ResponseData(Response::REDUDANT_DATA);

	if (mutData.havePermission())
		if (!CmdInterpreter::isValid(mutData.permission(), maxPriv))
			return ResponseData(Response::NO_PRIVILAGE);

	_initialize(mutData, maxPriv);
	return ResponseData(Response::SUCCESS, _policy, _timeToLive);
}
