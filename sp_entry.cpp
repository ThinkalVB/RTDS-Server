#include "sp_entry.h"

const std::string EntryV4::versionID = "v4";
const std::string EntryV6::versionID = "v6";

void entryBase::attachToPeer()
{
	iswithPeer = true;
}

void entryBase::detachFromPeer()
{
	iswithPeer = false;
}
