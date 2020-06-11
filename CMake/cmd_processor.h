#ifndef CMD_PROCESSOR_H
#define CMD_PROCESSOR_H

#include "peer.h"

struct CmdProcessor
{
	static const std::string RESP[];			// All response string
	static const std::string COMM[];			// All command string
/*******************************************************************************************
* @brief Process the commands from peer system
*
* @param[in]			The peer system from which the command is comming.
********************************************************************************************/
	static void processCommand(Peer&);
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
* @brief Check if the string is a valid Broadcast message
*
* @param[in]			The string view of the Bmessage.
* @return				True if the strig view is a Bmessage.
********************************************************************************************/
	static bool isBmessage(const std::string_view&);
/*******************************************************************************************
* @brief Check if the string view contains characters which are printable except space
*
* @param[in]			Command string.
* @return				True if printable
*
* @details
* Return true for empty strings.
********************************************************************************************/
	static bool isPrintable(const std::string_view&);
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

private:
/*******************************************************************************************
* @brief Respond to the ping request
*
* @param[in]			Peer.
*
* @details
* Call the appropriate peer functions based on the commands and parameters
********************************************************************************************/
	static void _cmd_ping(Peer&);
	static void _cmd_change(Peer&);
	static void _cmd_broadcast(Peer&);
	static void _cmd_exit(Peer&);
	static void _cmd_listen(Peer&);
	static void _cmd_leave(Peer&);
};

#endif