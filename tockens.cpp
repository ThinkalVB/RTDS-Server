#include "tockens.h"

AdvancedTocken* Tocken::makeUpdateTocken(BaseEntry* entry, BaseEntry* cmdEntry)
{
	auto maxPrivilege = entry->maxPrivilege(cmdEntry);
	if (entry->canChangeWith(maxPrivilege))
	{
		auto updateTocken = new AdvancedTocken;
		updateTocken->EvB = entry;
		updateTocken->maxPrivilege = maxPrivilege;
		entry->lock();
		return updateTocken;
	}
	else
		return nullptr;
}

AdvancedTocken* Tocken::makeChargeTocken(BaseEntry* entry, BaseEntry* cmdEntry)
{
	auto maxPrivilege = entry->maxPrivilege(cmdEntry);
	if (entry->canChargeWith(maxPrivilege))
	{
		auto chargeTocken = new AdvancedTocken;
		chargeTocken->EvB = entry;
		chargeTocken->maxPrivilege = maxPrivilege;
		entry->lock();
		return chargeTocken;
	}
	else
		return nullptr;
}

AdvancedTocken* Tocken::makeInsertionTocken(BaseEntry* entry, BaseEntry* cmdEntry)
{
	auto insertionTocken = new AdvancedTocken;
	insertionTocken->EvB = entry;
	insertionTocken->maxPrivilege = entry->maxPrivilege(cmdEntry);
	entry->lock();
	return insertionTocken;
}

BasicTocken* Tocken::makePurgeTocken(BaseEntry* entry, BaseEntry* cmdEntry)
{
	auto maxPrivilege = entry->maxPrivilege(cmdEntry);
	if (entry->canRemoveWith(maxPrivilege))
	{
		auto purgeTocken = new BasicTocken;
		purgeTocken->EvB = entry;
		entry->lock();
		return purgeTocken;
	}
	else
		return nullptr;
}

void Tocken::destroyTocken(BasicTocken* tocken)
{
	auto entry = tocken->EvB;
	entry->unlock();
	delete tocken;
}

void Tocken::destroyTocken(AdvancedTocken* tocken)
{
	auto entry = tocken->EvB;
	entry->unlock();
	delete tocken;
}

bool AdvancedTocken::haveValid(const Permission& perm)
{
	if (maxPrivilege >= perm.change && maxPrivilege >= perm.remove && maxPrivilege >= perm.charge)
		return true;
	else
		return false;
}
