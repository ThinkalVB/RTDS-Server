#ifndef MESSAGE_H
#define MESSAGE_H

#include "sapair.h"
#include <chrono>
#include <queue>
#include "common.h"

typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;

class Message
{
	static std::queue<Message*> mMessageQ;		// Concurrent message Queue
	static std::mutex mQinsLock;				// Message Q insertion lock
	static std::mutex mQremLock;				// Message Q removal lock

	TimePoint mCreatedTime;						// Message created time
/*******************************************************************************************
* @brief Delete all expired messages from the message queue
********************************************************************************************/
	static void mCleanMessageQ();
/*******************************************************************************************
* @brief Message constructor
*
* @param[in]		Message.
* @param[in]		Peers Tag.
*
* @details
* Initialize the values of createdTime, message and peers tag.
********************************************************************************************/
	//Message(const std::string&);
	//Message(const std::string&, const BGT&, const PeerType);

	template<typename MessageStr, typename BGTstr>
	Message(const MessageStr&, const BGTstr&, const PeerType);
public:

	std::string messageBuf;						// Message string buffer
	BGT peerTag;								// Tag value of the Peer
	PeerType peerType;							// Type of peer generating this message
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
* @param[in]		Peers Tag.
* @return			Constant pointer to the message or nullptr.
********************************************************************************************/
	static const Message* makeAddMsg(const SApair&, const BGT&);
/*******************************************************************************************
* @brief Make new peer removal message
*
* @param[in]		Source address pair.
* @param[in]		Peers Tag.
* @return			Constant pointer to the message or nullptr.
********************************************************************************************/
	static const Message* makeRemMsg(const SApair&, const BGT&);
/*******************************************************************************************
* @brief Make a new broadcast message (For UDP clients)
*
* @param[in]		Source address pair string.
* @param[in]		Message.
* @param[in]		Peers Tag.
* @return			Constant pointer to the message or nullptr.
********************************************************************************************/
	static const Message* makeBrdMsg(const std::string, const std::string_view&, const BGT&);
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

template<typename MessageStr, typename BGTstr>
Message::Message(const MessageStr& messageStr, const BGTstr& pTag, const PeerType pType)
{
	DEBUG_LOG(Log::log("New message created: ", messageStr);)
	mCreatedTime = std::chrono::system_clock::now();
	messageBuf = messageStr;
	peersTag = pTag;
	peerType = pType;
}




#endif