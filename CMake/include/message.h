#ifndef MESSAGE_H
#define MESSAGE_H

#include "sapair.h"
#include <chrono>
#include <queue>

typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

class Message
{
	static std::queue<Message*> mMessageQ;				// Concurrent message Queue
	static std::mutex mQinsLock;						// Message Q insertion lock
	static std::mutex mQremLock;						// Message Q removal lock

	TimePoint mCreatedTime;								// Message created time
/*******************************************************************************************
* @brief Delete all expired messages from the message queue
********************************************************************************************/
	static void mCleanMessageQ();
/*******************************************************************************************
* @brief Message constructor
*
* @param[in]		Message.
*
* @details
* Initialize the values of createdTime amd message
********************************************************************************************/
	Message(const std::string&);
public:

	std::string messageBuf;								// Message string buffer
/*******************************************************************************************
* @brief Return true if the message have expired
*
* @return			True if the message have expired.
********************************************************************************************/
	bool haveExpired() const;
/*******************************************************************************************
* @brief Make new peer addition message
*
* @param[in]		Source address pair.
* @param[in]		Broadcast Group Tag.
* @return			Constant pointer to the message or nullptr.
********************************************************************************************/
	static const Message* makeAddMsg(const SApair&);
/*******************************************************************************************
* @brief Make new peer removal message
*
* @param[in]		Source address pair.
* @return			Constant pointer to the message or nullptr.
********************************************************************************************/
	static const Message* makeRemMsg(const SApair&);
/*******************************************************************************************
* @brief Make a new broadcast message
*
* @param[in]		Source address pair.
* @param[in]		Message
* @return			Constant pointer to the message or nullptr.
********************************************************************************************/
	static const Message* makeBrdMsg(const SApair&, const std::string_view&);
/*******************************************************************************************
* @brief Insert message to the the message Queue
*
* @param[in]		Message.
* @return			True if success
*
* @details
* Message is deleted if the push_back fails.
********************************************************************************************/
	static bool insertMssg2Q(Message*);
};

#endif