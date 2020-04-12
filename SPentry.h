#pragma once
#include "Enums.hpp"
#include <string>
#include <boost/asio.hpp>

using namespace boost;
class SPentryBase {
protected:
	SPentryBase() {}
	~SPentryBase() {}

	SourcePair sourcePair;
	Permission permission;

	std::string UID;
	std::string ipAddress;
	std::string portNumber;
	std::string description;

	unsigned int ttl;
	posix_time::ptime startTime;
};

class SPentryV4 : private SPentryBase
{
	static const std::string versionID;
public:
	SPentryV4(asio::ip::address_v4, unsigned short);
	friend class CmdInterpreter;
};


class SPentryV6 :private SPentryBase
{
	static const std::string versionID;
public:
	SPentryV6(asio::ip::address_v6, unsigned short);
	friend class CmdInterpreter;
};

union SPentry
{
	SPentryV4* SPv4;
	SPentryV6* SPv6;
};