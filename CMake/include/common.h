#ifndef COMMON_H
#define COMMON_H

#define RTDS_VERSION 1.1.0
#define RTDS_MAJOR 1
#define RTDS_MINOR 1
#define RTDS_PATCH 0

#define RTDS_DUAL_STACK					// Enable IPV6 support (use ::1 for local host)
#define RDTS_DEF_PORT 321				// Default RTDS port number
#define RTDS_DEF_CCM_PORT 322			// Default CCM port number
#define MAX_THREAD_COUNT 28				// Maximum Thread Count
#define MIN_THREAD_COUNT 4				// Minimum Thread Count
#define MAX_PORT_NUM_VALUE 65535		// Maximum value for port number

#ifndef NDEBUG
#define PRINT_DEBUG_LOG					// Print debug log to file
#define OUTPUT_DEBUG_LOG				// Print debug log to screen
#endif
#define PRINT_LOG						// Print critical console logs

#define ALL_TAG "*"						// Represent all tags in a BG
#define UDP_TAG "$"						// Default BGT for UDP peers
#define OWN_TAG "+"						// The same peer's tag


#define MIN_BGID_SIZE 2					// Minimum size of BGID
#define MAX_BGID_SIZE 128				// Maximum size of BGID

#define MIN_TAG_SIZE 2					// Minimum size of Tag
#define MAX_TAG_SIZE 32					// Maximum size of Tag

#define RTDS_BUFF_SIZE 512				// Maximum size of the readBuffer
#define MAX_BROADCAST_SIZE 256			// Maximum size of B data
#define MAX_MSG_CACHE_SIZE 128			// Maximum number of messages to be cached
#define MIN_MSG_KEEP_TIME 1				// Minimum number of minutes to keep the message

#define USRN_MAX_SIZE 15				// Maximum size of username
#define PASS_MAX_SIZE 30				// Maximum size of password
#define ROOT_USRN "admin"
#define ROOT_PASS "admin"

#include <string>
typedef std::string BGID;
typedef std::string BGT;
typedef std::string SAP;

/*******************************************************************************************
* @brief Enum class for Response
*
* @details
* ok_success		Command is successful.
* bad_command		Bad command.
* bad_param			Bad parameter provided with the command.
* wait_retry		Wait and try again after sometime.
* is_in_bg			Peer is already listening to a BG.
* not_in_bg			Peer is not listening to any BG.
* not_allowed		The operation is not allowed.
********************************************************************************************/
enum class Response
{
	SUCCESS,
	BAD_COMMAND,
	BAD_PARAM,
	WAIT_RETRY,
	IS_IN_BG,
	NOT_IN_BG,
	NOT_ALLOWED
};

/*******************************************************************************************
* @brief Enum class for Command
*
* @details
* broadcast			Broadcast a message to the BG.
* ping				Ping with the RTDS server.
* listen			Listen to the activities in a Broadcast Group.
* change			Change the Broadcast Group.
* leave				Stop listening the Broadcast group.
* change			Change the tag of the peer.
* exit				Terminate the connection.
* login				[CCM] Login to RTDS CCM
* abort				[CCM] Terminate the RTDS server
* status			[CCM] Status of the RTDS server
********************************************************************************************/
enum class Command
{
	BROADCAST,
	PING,
	LISTEN,
	LEAVE,
	CHANGE,
	EXIT,
	LOGIN,
	ABORT,
	STATUS
};

/*******************************************************************************************
* @brief Enum class for Peer listening mode
*
* @details
* listen			Listening to the activities in a Broadcast Group.
* hear				Hearing the activities in a Broadcast Group.
********************************************************************************************/
enum class PeerMode
{
	LISTEN,
	HEAR
};

/*******************************************************************************************
* @brief Enum class pEER TYPE
*
* @details
* TCP				Listening to the activities in a Broadcast Group.
* UDP				Hearing the activities in a Broadcast Group.
********************************************************************************************/
enum class PeerType
{
	TCP,
	UDP,
	SSL,
	NONE
};

#endif
