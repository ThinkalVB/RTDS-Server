#include "mutable_data.h"

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

const Permission& MutableData::permission() const
{
	return _permission;
}

void MutableData::setDescription(const std::string_view& desc)
{
	_description = desc;
	_haveDescription = true;
}

void MutableData::setPermission(const Permission& perm)
{
	_permission = perm;
	_havePermission = true;
}