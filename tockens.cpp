#include "tockens.h"
#include "cmd_interpreter.h"

AdvancedTocken* Tocken::makeUpdateTocken(Entry* entry, Entry* cmdEntry)
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

AdvancedTocken* Tocken::makeChargeTocken(Entry* entry, Entry* cmdEntry)
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

AdvancedTocken* Tocken::makeInsertionTocken(Entry* entry, Entry* cmdEntry)
{
	auto insertionTocken = new AdvancedTocken;
	insertionTocken->EvB = entry;
	insertionTocken->maxPrivilege = entry->maxPrivilege(cmdEntry);
	entry->lock();
	return insertionTocken;
}

BasicTocken* Tocken::makePurgeTocken(Entry* entry, Entry* cmdEntry)
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

std::string AdvancedTocken::makeInsertionNote()
{
	std::string notification = "[+] " + EvB->uid();
	notification += '\x1e';				//!< Record separator
	return notification;
}

std::string AdvancedTocken::makeUpdateNote(const MutableData& data)
{
	std::string notification = "[$] " + EvB->uid();
	if (data._havePermission)
		notification += " " + CmdInterpreter::toPermission(data._permission);
	if (data._haveDescription)
		notification += " " + std::string(data._description);

	notification += '\x1e';				//!< Record separator
	return notification;
}

std::string BasicTocken::makePurgeNote()
{
	std::string notification = "[-] " + EvB->uid();
	notification += '\x1e';				//!< Record separator
	return notification;
}

bool MutableData::isEmpty()
{
	return !(_haveDescription || _havePermission);
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
