#ifndef MESSAGE_H
#define MESSAGE_H

#include <chrono>
#include <queue>
#include <mutex>
#include <asio/buffer.hpp>
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
* @param[in]		Peer Type.
*
* @details
* Initialize the values of createdTime, message, tags, peer type.
********************************************************************************************/
	template<typename MessageStr, typename BGTstr>
	Message(const MessageStr& mssgStr, const BGTstr& rTag, const PeerType& pType)
	{
		mCreatedTime = std::chrono::system_clock::now();
		messageBuf = mssgStr;
		peerType = pType;
		recverTag = rTag;
		asioBuffer = asio::mutable_buffer(messageBuf.data(), messageBuf.size())
	}
public:

	std::string messageBuf;						// Message string buffer
	BGT recverTag;								// Receivers tag
	PeerType peerType;							// Type of peer generating this message
	asio::mutable_buffer asioBuffer;
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
* @param[in]		Senders Tag.
* @param[in]		Receivers Tag.
* @param[in]		Peer Type.
* @return			Constant pointer to the message or nullptr.
********************************************************************************************/
	template<typename BGTstr>
	static const Message* makeAddMsg(const SAP& sapStr, const BGTstr& rTag, const PeerType pType)
	{
		std::string addMssg;
		if (pType == PeerType::TCP)
			addMssg = "[CT]";
		else if (pType == PeerType::SSL)
			addMssg = "[CS]";
		else if (pType == PeerType::UDP)
			addMssg = "[CU]";

		addMssg += "\t" + sapStr + "\n";
		auto message = new (std::nothrow) Message(addMssg, rTag, pType);
		if (message == nullptr)
				return nullptr;
		else
		{
			if (!insertMssg2Q(message))
				return nullptr;
			else
				return message;
		}
	}
/*******************************************************************************************
* @brief Make new peer removal message
*
* @param[in]		Source address pair.
* @param[in]		Peers Tag.
* @return			Constant pointer to the message or nullptr.
********************************************************************************************/
	template<typename BGTstr>
	static const Message* makeRemMsg(const SAP& sapStr, const BGTstr& rTag, const PeerType pType)
	{
		std::string remMssg;
		if (pType == PeerType::TCP)
			remMssg = "[DT]";
		else if (pType == PeerType::SSL)
			remMssg = "[DS]";
		else
			remMssg = "[DU]";

		remMssg += "\t" + sapStr + "\n";
		auto message = new (std::nothrow) Message(remMssg, rTag, pType);
		if (message == nullptr)
				return nullptr;
		else
		{
			if (!insertMssg2Q(message))
				return nullptr;
			else
				return message;
		}
	}
/*******************************************************************************************
* @brief Make a new broadcast message
*
* @param[in]		Source address pair string.
* @param[in]		Message.
* @param[in]		Senders Tag.
* @param[in]		Receivers Tag.
* @return			Constant pointer to the message or nullptr.
********************************************************************************************/
	template<typename MessageStr, typename BGTstr>
	static const Message* makeMsg(const SAP& sapStr, const MessageStr& mssgStr, const BGTstr& rTag, const PeerType pType)
	{
		std::string brdMssg;
		if (pType == PeerType::TCP)
			brdMssg = "[BT]";
		else if (pType == PeerType::SSL)
			brdMssg = "[BS]";
		else
			brdMssg = "[BU]";

		brdMssg += "\t" + sapStr + "\t";
		brdMssg += mssgStr;
		brdMssg += "\n";

		auto message = new (std::nothrow) Message(brdMssg, rTag, pType);
		if (message == nullptr)
			return nullptr;
		else
		{
			if (!insertMssg2Q(message))
				return nullptr;
			else
				return message;
		}
	}
/*******************************************************************************************
* @brief Make a new Message
*
* @param[in]		Message.
* @param[in]		Senders Tag.
* @param[in]		Receivers Tag.
* @return			Constant pointer to the message or nullptr.
********************************************************************************************/
	template<typename MessageStr, typename BGTstr>
	static const Message* makeBrdMsg(const MessageStr& mssgStr, const BGTstr& rTag, const PeerType pType)
	{
		std::string brdMssg;
		if (pType == PeerType::TCP)
			brdMssg = "[BT]";
		else if (pType == PeerType::SSL)
			brdMssg = "[BS]";
		else
			brdMssg = "[BU]";
		brdMssg += "\t";
		brdMssg += mssgStr;
		brdMssg += "\n";

		auto message = new (std::nothrow) Message(brdMssg, rTag, pType);
		if (message == nullptr)
			return nullptr;
		else
		{
			if (!insertMssg2Q(message))
				return nullptr;
			else
				return message;
		}
	}
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