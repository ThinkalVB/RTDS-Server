#include "sp_entry.h"
#include "cmd_interpreter.h"
#include "cppcodec/base64_rfc4648.hpp"
#include <boost/date_time.hpp>

const std::string EntryV4::versionID = "v4";
const std::string EntryV6::versionID = "v6";

EntryV4::EntryV4(asio::ip::address_v4 ipAdd, unsigned short portNum)
{
	portNumber = std::to_string(portNum);
	ipAddress = ipAdd.to_string();

	CmdInterpreter::makeSourcePairV4(ipAdd, portNum, sourcePair.V4SP);
	UID = cppcodec::base64_rfc4648::encode(sourcePair.V4SP, 6);

}

EntryV6::EntryV6(asio::ip::address_v6 ipAdd, unsigned short portNum)
{
	portNumber = std::to_string(portNum);
	ipAddress = ipAdd.to_string();

	CmdInterpreter::makeSourcePairV6(ipAdd, portNum, sourcePair.V6SP);
	UID = cppcodec::base64_rfc4648::encode(sourcePair.V6SP, 18);

}