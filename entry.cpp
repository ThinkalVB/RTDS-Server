#include "entry.h"
#include "cmd_interpreter.h"

unsigned int Entry::_tmAfterLastChrg() const
{
	auto timeNow = posix_time::second_clock::universal_time();
	auto diffTime = timeNow - _lastChargT;
	return (unsigned short)diffTime.minutes();
}

Response Entry::_charge()
{
	if (_isInDirectory)
	{
		auto timePassed = _tmAfterLastChrg();
		_lastChargT = posix_time::second_clock::universal_time();
		_timeToLive += timePassed;

		if (_timeToLive > MAX_TTL)
			_timeToLive = MAX_TTL;
		_timeLeft = _timeToLive;
		return Response::SUCCESS;
	}
	else
		return Response::NO_EXIST;
}

void Entry::_updateTimeLeft()
{
	auto timePassed = _tmAfterLastChrg();
	if (timePassed > _timeToLive)
	{
		_isInDirectory = false;
		_timeLeft = 0;
	}
	else
		_timeLeft = _timeToLive - timePassed;
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
	_isInDirectory = true;
}

Entry::Entry(const SPaddress& spAddr, const MutableData& mutData, const Privilege maxPriv) : _spAddress(spAddr)
{
	_uid = _spAddress.toUID();
	_portNumber = _spAddress.portNumber();
	_ipAddress = _spAddress.ipAddress();

	_isInDirectory = false;
	_iswithPeer = false;
}


ResponsePair Entry::makeEntry(const SPaddress& targetSPA, const SPaddress& cmdSPA, const MutableData& mutData)
{
	auto maxPriv = targetSPA.maxPrivilege(cmdSPA);
	if (mutData.havePermission())
		if (!CmdInterpreter::isValid(mutData.permission(), maxPriv))
			return std::make_pair(Response::NO_PRIVILAGE, nullptr);

	auto entry = new Entry(targetSPA, mutData, maxPriv);
	entry->_initialize(mutData, maxPriv);
	return std::make_pair(Response::SUCCESS, entry);
}

bool Entry::expired()
{
	std::lock_guard<std::mutex> lock(_entryLock);
	_updateTimeLeft();
	if (_isInDirectory)
		return false;
	else
		return true;
}

ResponseTTL Entry::tryGetTTL()
{
	std::lock_guard<std::mutex> lock(_entryLock);
	_updateTimeLeft();

	if (_isInDirectory)
		return std::make_pair(Response::SUCCESS, _timeLeft);
	else
		return std::make_pair(Response::NO_EXIST, _timeLeft);
}

const std::string& Entry::uid() const
{
	return _uid;
}

void Entry::printExpand(std::string& writeBuffer)
{
	printBrief(writeBuffer);

	std::lock_guard<std::mutex> lock(_entryLock);
	writeBuffer += CmdInterpreter::toPermission(_policy.permission());
	writeBuffer += " " + _policy.description();
}

void Entry::printBrief(std::string& writeBuffer) const
{
	if (_spAddress.version() == Version::V4)
		writeBuffer += STR_V4;
	else
		writeBuffer += STR_V6;
	writeBuffer += " " + _uid + " " + _ipAddress + " " + _portNumber + " ";
}

bool Entry::compatibleMD(const MutableData& policyMD)
{
	std::lock_guard<std::mutex> lock(_entryLock);
	_updateTimeLeft();

	if (_isInDirectory)
	{
		if (policyMD.description() == _policy.description() &&
			policyMD.permission() == _policy.permission())
			return true;
		else
			return false;
	}
	else
		return false;
}

ResponseTTL Entry::tryChargeWith(const SPaddress& spAddr)
{
	auto maxPriv = _spAddress.maxPrivilege(spAddr);
	std::lock_guard<std::mutex> lock(_entryLock);
	_updateTimeLeft();

	if (_policy.canChargeWith(maxPriv))
		return std::make_pair(_charge(), _timeLeft);
	else
		return std::make_pair(Response::NO_PRIVILAGE, _timeLeft);
}

Response Entry::tryRemoveWith(const SPaddress& spAddr)
{
	auto maxPriv = _spAddress.maxPrivilege(spAddr);
	std::lock_guard<std::mutex> lock(_entryLock);
	_updateTimeLeft();

	if (_isInDirectory)
	{
		if (_policy.canRemoveWith(maxPriv))
		{
			_isInDirectory = false;
			return Response::SUCCESS;
		}
		else
			return Response::NO_PRIVILAGE;
	}
	else
		return Response::NO_EXIST;
}

Response Entry::tryUpdateEntry(const SPaddress& spAddr, const MutableData& data)
{
	auto maxPriv = _spAddress.maxPrivilege(spAddr);
	std::lock_guard<std::mutex> lock(_entryLock);
	_updateTimeLeft();

	if (_isInDirectory)
	{
		if (_policy.canUpdateWith(maxPriv))
		{
			if (data.havePermission())
			{
				if (CmdInterpreter::isValid(data.permission(), maxPriv))
					_policy.setPermission(data.permission());
				else
					return Response::NO_PRIVILAGE;
			}

			if (data.haveDescription())
				_policy.setDescription(data.description());
			return Response::SUCCESS;
		}
		else
			return Response::NO_PRIVILAGE;
	}
	else
		return Response::NO_EXIST;
}

Response Entry::tryAddEntry(const SPaddress& cmdSPA, const MutableData& mutData)
{
	auto maxPriv = _spAddress.maxPrivilege(cmdSPA);
	std::lock_guard<std::mutex> lock(_entryLock);
	_updateTimeLeft();

	if (_isInDirectory)
		return Response::REDUDANT_DATA;

	if (mutData.havePermission())
		if (!CmdInterpreter::isValid(mutData.permission(), maxPriv))
			return Response::NO_PRIVILAGE;

	_initialize(mutData, maxPriv);
	return Response::SUCCESS;
}
