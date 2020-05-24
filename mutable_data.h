#ifndef MUTABLE_DATA_H
#define MUTABLE_DATA_H

#include "common.hpp"

class _perm_data
{
protected:
	Permission _permission;

public:
	void setPermission(const Permission&);
	const Permission& permission() const;
	bool canRemoveWith(const Privilege&) const;
	bool canUpdateWith(const Privilege&) const;
	bool canChargeWith(const Privilege&) const;
};

class MutableData : public _perm_data
{
	bool _havePermission = false;
	bool _haveDescription = false;
	std::string_view _description;

public:
	bool isEmpty() const;
	bool havePermission() const;
	bool haveDescription() const;
	bool isValidPolicy() const;

	const std::string_view& description() const;
	void setDescription(const std::string_view&);
};

class Policy : public _perm_data
{
	std::string _description;

public:
	Policy();
	Policy(const Permission& perm, const std::string_view& desc);

	const std::string& description() const;
	void setDescription(const std::string_view&);
	void operator = (const MutableData&);
};
#endif

