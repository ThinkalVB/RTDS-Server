#pragma once
#include <cstdint>
#include <string>

enum class Permission : uint8_t
{
	LIBERAL_ENTRY,
	PROTECTED_ENTRY,
	RESTRICTED_ENTRY
};

union SourcePair
{
	uint8_t V4SP[6];
	uint8_t V6SP[18];
};

namespace Response{
	const std::string REDUDANT_DATA =	"redudant_data";
	const std::string SUCCESS		=	"ok_success";
	const std::string CHOOSE_DIR	=	"choose_dir";
	const std::string NO_PRIVILAGE	=	"no_privilege";
	const std::string BAD_COMMAND	=	"bad_command";
	const std::string BAD_PARAM		=	"bad_param";
	const std::string NO_EXIST		=	"no_exist";
	const std::string WAIT_RETRY	=	"wait_retry";
}

namespace Command {
	const std::string COM_PING		= "ping";
	const std::string COM_REGISTER	= "register";
	const std::string COM_SWITCH	= "switch";
	const std::string COM_ADD		= "add";
	const std::string COM_SEARCH	= "search";
	const std::string COM_TTL		= "ttl";
	const std::string COM_CHARGE	= "charge";
	const std::string COM_UPDATE	= "update";
	const std::string COM_REMOVE	= "remove";
	const std::string COM_COUNT		= "count";
	const std::string COM_MIRROR	= "mirror";
	const std::string COM_LEAVE		= "leave";
	const std::string COM_EXIT		= "exit";
	const std::string COM_DELETE	= "no_exist";
}