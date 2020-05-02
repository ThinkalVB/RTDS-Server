#ifndef TOCKENS_H
#define TOCKENS_H

#include "sp_entry.h"

struct MutableData
{
	std::string_view description;
	Permission permission;

	bool havePermission = false;
	bool haveDescription = false;
	bool isEmpty()
	{
		return !(haveDescription || havePermission);
	}
};

class BasicTocken
{
	BasicTocken() {}
	BaseEntry* EvB = nullptr;
public:
	std::string makePurgeNote();
	friend class Tocken;
	friend class Directory;
};
typedef BasicTocken PurgeTocken;

class AdvancedTocken
{
	AdvancedTocken() {}
	BaseEntry* EvB = nullptr;
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
	static AdvancedTocken* makeUpdateTocken(BaseEntry*, BaseEntry*);
	static AdvancedTocken* makeChargeTocken(BaseEntry*, BaseEntry*);
	static AdvancedTocken* makeInsertionTocken(BaseEntry*, BaseEntry*);
	static BasicTocken* makePurgeTocken(BaseEntry*, BaseEntry*);

	static void destroyTocken(BasicTocken*);
	static void destroyTocken(AdvancedTocken*);
};

#endif
