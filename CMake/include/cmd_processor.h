#ifndef CMD_PROCESSOR_H
#define CMD_PROCESSOR_H

#include "tcp_peer.h"
#include "udp_peer.h"
#include "ssl_ccm.h"
#include "ssl_peer.h"

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
	static void processCommand(SSLccm&);
	static void processCommand(UDPpeer&);
	template<typename TSpeer>
	static void processCommand(TSpeer& peer)
	{
		auto commandStr = peer.getCommandString();
		auto command = extractElement(commandStr);

		if (command == COMM[(short)Command::BROADCAST])
			mTCP_broadcast(peer, commandStr);
		else if (command == COMM[(short)Command::MESSAGE])
			mTCP_message(peer, commandStr);
		else if (command == COMM[(short)Command::CHANGE])
			mTCP_change(peer, commandStr);
		else if (command == COMM[(short)Command::LISTEN])
			mTCP_listen(peer, commandStr);
		else if (command == COMM[(short)Command::LEAVE])
			mTCP_leave(peer, commandStr);
		else if (command == COMM[(short)Command::PING])
			mTCP_ping(peer, commandStr);
		else if (command == COMM[(short)Command::EXIT])
			mTCP_exit(peer, commandStr);
		else
			peer.respondWith(Response::BAD_COMMAND);
	}

/*******************************************************************************************
* @brief Return the tag type
*
* @param[in]			The string view of the Tag.
* @return				Return the tag's type.
********************************************************************************************/
	static TagType getTagType(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string is a valid Tag
*
* @param[in]			The string view of the Tag.
* @return				True if the strig view is a Tag.
********************************************************************************************/
	static bool isTag(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string is a valid Tag
*
* @param[in]			The string view of the Tag.
* @return				True if the strig view is a Tag.
********************************************************************************************/
	static bool isGeneralTag(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string is a valid Broadcast Group ID
*
* @param[in]			The string view of the BGDI.
* @return				True if the strig view is a BGID.
********************************************************************************************/
	static bool isBGID(const std::string_view&);
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
* @brief Check if the string view contains characters which are printable (including space)
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

/*******************************************************************************************
* @brief Respond to the request
*
* @param[in]			Peer.
* @param[in]			Rest of the command string.
*
* @details
* Call the appropriate peer functions based on the commands and parameters
********************************************************************************************/
	template<typename TSpeer>
	static void mTCP_ping(TSpeer& peer, std::string_view& commandStr)
	{
		if (commandStr.empty())
			peer.printPingInfo();
		else
			peer.respondWith(Response::BAD_PARAM);
	}
	template<typename TSpeer>
	static void mTCP_broadcast(TSpeer& peer, std::string_view& commandStr)
	{
		auto message = extractElement(commandStr);
		auto bgTag = extractElement(commandStr);

		if (isBmessage(message) && commandStr.empty())
			peer.broadcastTo(message, bgTag);
		else
			peer.respondWith(Response::BAD_PARAM);
	}
	template<typename TSpeer>
	static void mTCP_message(TSpeer& peer, std::string_view& commandStr)
	{
		auto message = extractElement(commandStr);
		auto bgTag = extractElement(commandStr);

		if (isBmessage(message) && commandStr.empty())
			peer.broadcastTo(message, bgTag);
		else
			peer.respondWith(Response::BAD_PARAM);
	}
	template<typename TSpeer>
	static void mTCP_exit(TSpeer& peer, std::string_view& commandStr)
	{
		if (commandStr.empty())
			peer.disconnect();
		else
			peer.respondWith(Response::BAD_PARAM);
	}
	template<typename TSpeer>
	static void mTCP_listen(TSpeer& peer, std::string_view& commandStr)
	{
		auto bgID = extractElement(commandStr);
		auto bgTag = extractElement(commandStr);
		if (isGeneralTag(bgTag) && isBGID(bgID) && commandStr.empty())
			peer.listenTo(bgID, bgTag);
		else
			peer.respondWith(Response::BAD_PARAM);
	}
	template<typename TSpeer>
	static void mTCP_change(TSpeer& peer, std::string_view& commandStr)
	{
		auto bgTag = extractElement(commandStr);
		if (isGeneralTag(bgTag) && commandStr.empty())
			peer.changeTag(bgTag);
		else
			peer.respondWith(Response::BAD_PARAM);
	}
	template<typename TSpeer>
	static void mTCP_leave(TSpeer& peer, std::string_view& commandStr)
	{
		if (commandStr.empty())
			peer.leaveBG();
		else
			peer.respondWith(Response::BAD_PARAM);
	}
};

#endif
