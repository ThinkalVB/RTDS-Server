#include "mutable_data.h"

void _perm_data::setPermission(const Permission& perm)
{
	_permission = perm;
}

const Permission& _perm_data::permission() const
{
	return _permission;
}

bool _perm_data::canRemoveWith(const Privilege& maxPriv) const
{
	if (maxPriv >= _permission.remove)
		return true;
	else
		return false;
}

bool _perm_data::canUpdateWith(const Privilege& maxPriv) const
{
	if (maxPriv >= _permission.change)
		return true;
	else
		return false;
}

bool _perm_data::canChargeWith(const Privilege& maxPriv) const
{
	if (maxPriv >= _permission.charge)
		return true;
	else
		return false;
}


bool MutableData::isEmpty() const
{
	return !(_haveDescription || _havePermission);
}

bool MutableData::havePermission() const
{
	return _havePermission;
}

bool MutableData::haveDescription() const
{
	return _haveDescription;
}

bool MutableData::isValidPolicy() const
{
	if (_haveDescription && _havePermission)
		return true;
	else
		return false;
}

const std::string_view& MutableData::description() const
{
	return _description;
}

void MutableData::setDescription(const std::string_view& desc)
{
	_description = desc;
	_haveDescription = true;
}


Policy::Policy()
{
	_description = "[]";
	_permission.charge = Privilege::LIBERAL_ENTRY;
	_permission.change = Privilege::LIBERAL_ENTRY;
	_permission.remove = Privilege::LIBERAL_ENTRY;
}

Policy::Policy(const Permission& perm, const std::string_view& desc)
{
	_permission = perm;
	_description = desc;
}

const std::string& Policy::description() const
{
	return _description;
}

void Policy::setDescription(const std::string_view& desc)
{
	_description = desc;
}

void Policy::operator=(const MutableData& mutData)
{
	if (mutData.haveDescription())
		_description = mutData.description();
	if (mutData.havePermission())
		_permission = mutData.permission();
}
