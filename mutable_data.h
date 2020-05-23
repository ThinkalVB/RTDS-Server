#ifndef MUTABLE_DATA_H
#define MUTABLE_DATA_H

#include "common.hpp"

class MutableData
{
	bool _havePermission = false;
	bool _haveDescription = false;

	std::string_view _description;
	Permission _permission;

public:
	bool isEmpty() const;
	bool havePermission() const;
	bool haveDescription() const;
	bool isValidPolicy() const;

	const std::string_view& description() const;
	const Permission& permission() const;
	void setDescription(const std::string_view&);
	void setPermission(const Permission&);
};
typedef MutableData Policy;
#endif

