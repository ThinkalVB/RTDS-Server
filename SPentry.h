#pragma once
#include "Enums.h"
#include <string>
#include <boost/asio.hpp>

using namespace boost;

class SPentryV4
{
	SourcePair sourcePair;
	Permission permission;

	static const std::string versionID;
	std::string UID;
	std::string ipAddress;
	std::string portNumber;
	std::string description;

	unsigned int ttl;
	posix_time::ptime startTime;
public:
	SPentryV4(asio::ip::address_v4, unsigned short);
};


class SPentryV6
{
	SourcePair sourcePair;
	Permission permission;

	static const std::string versionID;
	std::string UID;
	std::string ipAddress;
	std::string portNumber;
	std::string description;

	unsigned int ttl;
	posix_time::ptime startTime;
public:
	SPentryV6(asio::ip::address_v6, unsigned short);
};