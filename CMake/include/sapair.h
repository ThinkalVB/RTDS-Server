#ifndef SA_PAIR_H
#define SA_PAIR_H

#include <asio.hpp>
#include <string>

/*******************************************************************************************
 * @brief Class to hold both SourcePairV4 and SourcePairV6
 ********************************************************************************************/
class SApair
{
	asio::ip::address _ipAddr;			// IPaddress.
	unsigned short _portNumber;			// Associated port number.
	std::string _saPairStr;				// SApair string (Version+IPaddress+portNumber)

public:
/*******************************************************************************************
* @brief Constructor
*
* @param[in]			Asio pointer to the TCP socket.
********************************************************************************************/
	SApair(asio::ip::tcp::socket* tcpSocket);
/*******************************************************************************************
* @brief Convert the source address pair to string (Version, IP address, port number)
*
* @return				Source Address pair string.
********************************************************************************************/
	std::string toString() const;
/*******************************************************************************************
* @brief Return true 
*
* @param[in]			Source address pair.
* @return				True if the IP address and port number are the same.
********************************************************************************************/
	bool operator==(const SApair&) const;
};

#endif