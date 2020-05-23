#include "spaddress.h"
#include "cmd_interpreter.h"
#include <cppcodec/base64_rfc4648.hpp>
#include "entry.h"

SPaddress::SPaddress(const asio::ip::address_v4& ipAdd, unsigned short portNum)
{
	_SPA.IPV4 = ipAdd.to_bytes();
	_version = Version::V4;
	_portNumber = portNum;

	#ifdef BOOST_ENDIAN_LITTLE_BYTE
	CmdInterpreter::byteSwap(portNum);
	#endif
	memcpy(&_SPA.V4[_SPA.V4.size() - 2], &portNum, 2);
}

SPaddress::SPaddress(const asio::ip::address_v6& ipAdd, unsigned short portNum)
{
	_SPA.IPV6 = ipAdd.to_bytes();
	_version = Version::V6;
	_portNumber = portNum;

	#ifdef BOOST_ENDIAN_LITTLE_BYTE
	CmdInterpreter::byteSwap(portNum);
	#endif
	memcpy(&_SPA.V6[_SPA.V6.size() - 2], &portNum, 2);
}

SPaddress::SPaddress(const std::string_view& uid)
{
	if (uid.size() == V4_UID_MAX_CHAR)
	{
		cppcodec::base64_rfc4648::decode(_SPA.V4.data(), _SPA.V4.size(), uid);
		_version = Version::V4;

		unsigned short portNum;
		memcpy(&portNum, _SPA.V4.data() + _SPA.IPV4.size(), 2);
		#ifdef BOOST_ENDIAN_LITTLE_BYTE
		CmdInterpreter::byteSwap(portNum);
		#endif
		_portNumber = portNum;
	}
	else
	{
		cppcodec::base64_rfc4648::decode(_SPA.V6.data(), _SPA.V6.size(), uid);
		_version = Version::V6;

		unsigned short portNum;
		memcpy(&portNum, _SPA.V6.data() + _SPA.IPV6.size(), 2);
		#ifdef BOOST_ENDIAN_LITTLE_BYTE
		CmdInterpreter::byteSwap(portNum);
		#endif
		_portNumber = portNum;
	}
}

SPaddress::SPaddress(const asio::ip::tcp::endpoint& remoteEp)
{
	if (remoteEp.address().is_v4())
	{
		auto portNum = remoteEp.port();
		auto ipAdd = remoteEp.address().to_v4();

		_SPA.IPV4 = ipAdd.to_bytes();
		_version = Version::V4;
		_portNumber = portNum;

		#ifdef BOOST_ENDIAN_LITTLE_BYTE
		CmdInterpreter::byteSwap(portNum);
		#endif
		memcpy(&_SPA.V4[_SPA.V4.size() - 2], &portNum, 2);

	}
	else
	{
		auto portNum = remoteEp.port();
		auto ipAdd = remoteEp.address().to_v6();

		_SPA.IPV6 = ipAdd.to_bytes();
		_version = Version::V6;
		_portNumber = portNum;

		#ifdef BOOST_ENDIAN_LITTLE_BYTE
		CmdInterpreter::byteSwap(portNum);
		#endif
		memcpy(&_SPA.V6[_SPA.V6.size() - 2], &portNum, 2);
	}
}

std::string SPaddress::toUID() const
{
	if (_version == Version::V4)
		return cppcodec::base64_rfc4648::encode(_SPA.V4);
	else
		return cppcodec::base64_rfc4648::encode(_SPA.V6);
}

std::string SPaddress::ipAddress() const
{
	if (_version == Version::V4)
		return ((asio::ip::address_v4*) & _SPA.IPV4)->to_string();
	else
		return ((asio::ip::address_v6*) & _SPA.IPV6)->to_string();
}

std::string SPaddress::briefInfo() const
{
	std::string briefString;
	if (_version == Version::V4)
	{
		briefString += STR_V4;
		briefString += " " + ((asio::ip::address_v4*) & _SPA.IPV4)->to_string();
	}
	else
	{
		briefString += STR_V6;
		briefString += " " + ((asio::ip::address_v6*) & _SPA.IPV6)->to_string();
	}
	briefString += " " + portNumber();
	return briefString;
}

std::string SPaddress::portNumber() const
{
	return std::to_string(_portNumber);
}

Version SPaddress::version() const
{
	return _version;
}

const SourcePairV4& SPaddress::spAddressV4() const
{
	return _SPA.V4;
}

const SourcePairV6& SPaddress::spAddressV6() const
{
	return _SPA.V6;
}

Privilege SPaddress::maxPrivilege(const SPaddress& spAddress) const
{
	if (_version != spAddress._version)
		return Privilege::LIBERAL_ENTRY;
	
	if (_version == Version::V4)
	{
		if (_SPA.IPV4 != spAddress._SPA.IPV4)
			return Privilege::LIBERAL_ENTRY;
	}
	else
	{
		if (_SPA.IPV6 != spAddress._SPA.IPV6)
			return Privilege::LIBERAL_ENTRY;
	}

	if (_portNumber == spAddress._portNumber)
		return Privilege::RESTRICTED_ENTRY;
	else
		return Privilege::PROTECTED_ENTRY;
}