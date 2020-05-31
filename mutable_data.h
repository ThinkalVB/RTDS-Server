#ifndef MUTABLE_DATA_H
#define MUTABLE_DATA_H

#include "common.hpp"
#include <string_view>

class _perm_data
{
protected:
	Permission _permission;

public:
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

	void setPermission(const Permission&);
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
	void setPermission(const Permission&);
	void setDescription(const std::string_view&);
	void printPolicy(std::string& writeBuffer) const;
	void operator=(const MutableData&);
	bool operator==(const Policy&);
};

class ResponseData
{
	Response _response;
	Policy _policy;
	unsigned int _ttl;
public:
	ResponseData(const Response, const Policy&);
	ResponseData(const Response, const unsigned int ttl);
	ResponseData(const Response);

	bool operator==(const Response) const;
	void printPolicy(std::string&) const;
	void printResponse(std::string&) const;
	void printTTL(std::string&) const;
	const Policy& policy() const;
};
#endif

