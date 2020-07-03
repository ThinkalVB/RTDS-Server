#ifndef CMD_PROCESSOR_H
#define CMD_PROCESSOR_H

#include "tcp_peer.h"
#include "udp_peer.h"
#include "ssl_peer.h"

struct CmdProcessor
{
	static const std::string RESP[];			// All response string
	static const std::string COMM[];			// All command string
/*******************************************************************************************
* @brief Process the commands from peer system
*
* @param[in]			The peer system from which the command is comming.
********************************************************************************************/
	static void processCommand(TCPpeer&);
	static void processCommand(SSLpeer&);
/*******************************************************************************************
* @brief Process the commands from Receive Buffer and return response
*
* @param[in]			The peer system from which the command is comming.
* @param[in]			Receive buffer
********************************************************************************************/
	static void processCommand(UDPpeer&, AdancedBuffer&);
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
	static bool isTag(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string is a valid Tag (includes * tag)
*
* @param[in]			The string view of the Tag.
* @return				True if the strig view is a Wildchar Tag.
********************************************************************************************/
	static bool isWTag(const std::string_view&);
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
	static bool isUsername(const std::string_view&);
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
	static void mTCP_exit(TCPpeer&, std::string_view&);
	static void mTCP_listen(TCPpeer&, std::string_view&);
	static void mTCP_change(TCPpeer&, std::string_view&);
	static void mTCP_hear(TCPpeer&, std::string_view&);
	static void mTCP_leave(TCPpeer&, std::string_view&);

/*******************************************************************************************
* @brief Respond to the request
*
* @param[in]			Peer.
* @param[in]			Rest of the command string.
* @param[in]			Response Buffer
*
* @details
* Call the appropriate peer functions based on the commands and parameters.
* Response to the command will be copied back to the Advanced buffer.
********************************************************************************************/
	static void mUDP_ping(UDPpeer&, std::string_view&, AdancedBuffer&);
	static void mUDP_broadcast(UDPpeer&, std::string_view&, AdancedBuffer&);

/*******************************************************************************************
* @brief Respond to the ping request
*
* @param[in]			Peer.
* @param[in]			Rest of the command string.
*
* @details
* Call the appropriate peer functions based on the commands and parameters
********************************************************************************************/
	static void mSSL_login(SSLpeer&, std::string_view&);
	static void mSSL_exit(SSLpeer&, std::string_view&);
	static void mSSL_status(SSLpeer&, std::string_view&);
	static void mSSL_abort(SSLpeer&, std::string_view&);
};

#endif
