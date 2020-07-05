#ifndef CMD_PROCESSOR_H
#define CMD_PROCESSOR_H

#include "tcp_peer.h"
#include "udp_peer.h"
#include "ssl_ccm.h"

#define STR_V4 "v4"						// Version V4 in string
#define STR_V6 "v6"						// Version V6 in string

struct CmdProcessor
{
	static const std::string RESP[];	// All response string
	static const std::string COMM[];	// All command string
/*******************************************************************************************
* @brief Process the commands from peer system
*
* @param[in]			The peer system from which the command is comming.
********************************************************************************************/
	static void processCommand(TCPpeer&);
	static void processCommand(SSLccm&);
	static void processCommand(UDPpeer&);
/*******************************************************************************************
* @brief Check if the string is a valid Broadcast Group ID
*
* @param[in]			The string view of the BGDI.
* @return				True if the strig view is a BGID.
********************************************************************************************/
	static bool isBGID(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string is a valid Tag
*
* @param[in]			The string view of the Tag.
* @return				True if the strig view is a Tag.
********************************************************************************************/
	static bool isGeneralTag(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string is a UDP compatible Tag (General tag + "*")
*
* @param[in]			The string view of the Tag.
* @return				True if the strig view is a UDP compatible Tag.
********************************************************************************************/
	static bool isUDPcompatibleTag(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string is a Broadcast Tag (General tag + "*" + "+")
*
* @param[in]			The string view of the Tag.
* @return				True if the strig view is a UDP compatible Tag.
********************************************************************************************/
	static bool isBroadcastTag(const std::string_view&);
/*******************************************************************************************
* @brief Extract the next element from the command string.
*
* @param[in]			Command string.
* @return				The next element.
*
* @details
* This function will trim the extracted element from the command string.
* Return empty string view if no elements are to be found.
********************************************************************************************/
	static const std::string_view extractElement(std::string_view&);
/*******************************************************************************************
* @brief Check if the string is a valid Broadcast message
*
* @param[in]			The string view of the Bmessage.
* @return				True if the strig view is a Bmessage.
********************************************************************************************/
	static bool isBmessage(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string view contains characters which are printable (except space)
*
* @param[in]			Command string.
* @return				True if printable
*
* @details
* Return true for empty strings.
********************************************************************************************/
	static bool isConsistent(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string view contains characters which are printable
*
* @param[in]			Command string.
* @return				True if printable
*
* @details
* Return true for empty strings.
********************************************************************************************/
	static bool isPrintable(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string view is a username
*
* @param[in]			Command string.
* @return				True if username
********************************************************************************************/
	static bool isUsername(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string view is a password
*
* @param[in]			Command string.
* @return				True if password
********************************************************************************************/
	static bool isPassword(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string is a port number
*
* @param[in]			Port number.
* @param[out]			Port number value if true.
* @return				True if port number.
********************************************************************************************/
	static bool isPortNumber(const std::string,unsigned short&);
/*******************************************************************************************
* @brief Check if the string is a thread count
*
* @param[in]			Thread count.
* @param[out]			Thread count value if true.
* @return				True if thread count.
********************************************************************************************/
	static bool isThreadCount(const std::string, short&);
/*******************************************************************************************
* @brief get SAP string
*
* @param[in]			Remote Endpoint.
* @return				SAP string.
********************************************************************************************/
	template<typename ASIOep>
	static std::string getSAPstring(const ASIOep& remoteEp)
	{
		SAP sapString;
		auto ipAddr = remoteEp.address();
		auto portNumber = remoteEp.port();

		auto ipAddr6 = ipAddr.to_v6();
		if (ipAddr6.is_v4_mapped() || ipAddr6.is_v4_compatible())
		{
			sapString += STR_V4;
			sapString += "\t" + ipAddr6.to_v4().to_string();
		}
		else
		{
			sapString += STR_V6;
			sapString += "\t" + ipAddr6.to_string();
		}
		sapString += "\t" + std::to_string(portNumber);
		return sapString;
	}

private:
/*******************************************************************************************
* @brief Respond to the request
*
* @param[in]			Peer.
* @param[in]			Rest of the command string.
*
* @details
* Call the appropriate peer functions based on the commands and parameters
********************************************************************************************/
	static void mTCP_ping(TCPpeer&, std::string_view&);
	static void mTCP_broadcast(TCPpeer&, std::string_view&);
	static void mTCP_message(TCPpeer&, std::string_view&);
	static void mTCP_exit(TCPpeer&, std::string_view&);
	static void mTCP_listen(TCPpeer&, std::string_view&);
	static void mTCP_change(TCPpeer&, std::string_view&);
	static void mTCP_leave(TCPpeer&, std::string_view&);

/*******************************************************************************************
* @brief Respond to the request
*
* @param[in]			Peer.
* @param[in]			Rest of the command string.
*
* @details
* Call the appropriate peer functions based on the commands and parameters.
********************************************************************************************/
	static void mUDP_ping(UDPpeer&, std::string_view&);
	static void mUDP_broadcast(UDPpeer&, std::string_view&);
	static void mUDP_message(UDPpeer&, std::string_view&);

/*******************************************************************************************
* @brief Respond to the ping request
*
* @param[in]			Peer.
* @param[in]			Rest of the command string.
*
* @details
* Call the appropriate peer functions based on the commands and parameters
********************************************************************************************/
	static void mCCM_login(SSLccm&, std::string_view&);
	static void mCCM_exit(SSLccm&, std::string_view&);
	static void mCCM_status(SSLccm&, std::string_view&);
	static void mCCM_abort(SSLccm&, std::string_view&);
};

#endif
