#include "SPentry.h"
#include "CmdInterpreter.h"
#include "cppcodec/base64_rfc4648.hpp"
#include <boost/date_time.hpp>

const std::string SPentryV4::versionID = "v4";
const std::string SPentryV6::versionID = "v6";

SPentryV4::SPentryV4(asio::ip::address_v4 ipAdd, unsigned short portNum)
{
	startTime = posix_time::second_clock::local_time();
	portNumber = std::to_string(portNum);

	CmdInterpreter::makeSourcePairV4(ipAdd, portNum, sourcePair.V4SP);
	UID = cppcodec::base64_rfc4648::encode(sourcePair.V4SP, 6);
	ipAddress = ipAdd.to_string();


}

SPentryV6::SPentryV6(asio::ip::address_v6 ipAdd, unsigned short portNum)
{
	startTime = posix_time::second_clock::local_time();
	portNumber = std::to_string(portNum);

	CmdInterpreter::makeSourcePairV6(ipAdd, portNum, sourcePair.V6SP);
	UID = cppcodec::base64_rfc4648::encode(sourcePair.V6SP, 18);
	ipAddress = ipAdd.to_string();


}