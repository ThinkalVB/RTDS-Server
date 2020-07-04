#ifndef SA_PAIR_H
#define SA_PAIR_H

#include <asio/ip/tcp.hpp>
#include <string>

#define STR_V4 "v4"						// Version V4 in string
#define STR_V6 "v6"						// Version V6 in string

/*******************************************************************************************
 * @brief Class to hold both SourcePairV4 and SourcePairV6
 ********************************************************************************************/
class SApair
{
	std::string mSaPairStr;				// SApair string (Version+IPaddress+portNumber)
public:
/*******************************************************************************************
* @brief Constructor
*
* @param[in]			Asio pointer to the TCP socket.
********************************************************************************************/
	SApair::SApair(asio::ip::tcp::socket* tcpSocket) : SApair(tcpSocket->remote_endpoint()) {}
/*******************************************************************************************
* @brief Constructor
*
* @param[in]			Asio UDP/TCP endpoint.
********************************************************************************************/
	template<typename AsioEP>
	SApair(const AsioEP remoteEp)
	{
		auto ipAddr = remoteEp.address();
		auto portNumber = remoteEp.port();

		auto _ipAddr6 = ipAddr.to_v6();
		if (_ipAddr6.is_v4_mapped())
		{
			mSaPairStr += STR_V4;
			mSaPairStr += "\t" + _ipAddr6.to_v4().to_string();
		}
		else
		{
			mSaPairStr += STR_V6;
			mSaPairStr += "\t" + _ipAddr6.to_string();
		}
		mSaPairStr += "\t" + std::to_string(portNumber);
	}
/*******************************************************************************************
* @brief Convert the source address pair to string (Version, IP address, port number)
*
* @return				Source Address pair string.
********************************************************************************************/
	std::string toString() const;
};

#endif