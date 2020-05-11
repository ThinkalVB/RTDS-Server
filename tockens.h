#ifndef TOCKENS_H
#define TOCKENS_H

#include "entry.h"

class MutableData
{
	bool _havePermission = false;
	bool _haveDescription = false;

	std::string_view _description;
	Permission _permission;

public:
	bool isEmpty();

	void setDescription(const std::string_view&);
	void setPermission(const Permission&);
	friend class AdvancedTocken;
	friend class Directory;
};

class BasicTocken
{
	BasicTocken() {}
	Entry* EvB = nullptr;
public:
	std::string makePurgeNote();
	friend class Tocken;
	friend class Directory;
};
typedef BasicTocken PurgeTocken;

class AdvancedTocken
{
	AdvancedTocken() {}
	Entry* EvB = nullptr;
	Privilege maxPrivilege = Privilege::LIBERAL_ENTRY;
public:
/*******************************************************************************************
* @brief Return true if the permission is permitted for the tocken
*
* @param[in] perm				The permission to be checked for validity
* @return						True if the permission is valid
********************************************************************************************/
	bool haveValid(const Permission&);
	std::string makeInsertionNote();
	std::string makeUpdateNote(const MutableData&);
	friend class Tocken;
	friend class Directory;
};
typedef AdvancedTocken ChargeTocken;
typedef AdvancedTocken UpdateTocken;
typedef AdvancedTocken InsertionTocken;

class Tocken
{
public:
	static AdvancedTocken* makeUpdateTocken(Entry*, Entry*);
	static AdvancedTocken* makeChargeTocken(Entry*, Entry*);
	static AdvancedTocken* makeInsertionTocken(Entry*, Entry*);
	static BasicTocken* makePurgeTocken(Entry*, Entry*);

	static void destroyTocken(BasicTocken*);
	static void destroyTocken(AdvancedTocken*);
};

#endif
