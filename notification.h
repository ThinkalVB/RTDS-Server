#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "notification.h"
#include <string>
#include <list>
#include <mutex>
#include "entry.h"
#include "common.hpp"

struct Note
{
	std::string noteString;								//!< Notification string	
	int notificationNo;									//!< Notification number
	Note(const std::string& note, const int noteNo);
};

class Notification
{
	static int _notificationCount;						//!< Total number of notifications created
	static std::list<Note> _notificationList;			//!< List of notifications
	static std::mutex _NListLock;						//!< Notification list lock
/*******************************************************************************************
* @brief Return Notification
*
* @param[in] noteString			Notification string for which the notification is to be created.
* @return						Return the notification.
*
* @details
* Assign notification number and insert the notification into the list.
********************************************************************************************/
	static const Note& _newNote(const std::string&);
	Notification() = delete;

public:
/*******************************************************************************************
* @brief Make update Nottification
*
* @param[in] entry				Entry for which the notification is to be created.
* @param[in] mutData			Mutable data containing the changes in the entry.
* @return						Return the notification.
********************************************************************************************/
	static const Note& makeUpdateNote(const Entry*, const MutableData&);
/*******************************************************************************************
* @brief Make remove Nottification
*
* @param[in] entry				Entry for which the notification is to be created.
* @return						Return the notification.
********************************************************************************************/
	static const Note& makeAddNote(const Entry*);
/*******************************************************************************************
* @brief Make remove Nottification
*
* @param[in] spa				SPAddress of the entry being removed.
* @return						Return the notification.
********************************************************************************************/
	static const Note& makeRemoveNote(const Entry*);
};
#endif