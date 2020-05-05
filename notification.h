#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <string>
#include <list>
#include <mutex>
#include "common.hpp"

struct Note
{
	const std::string noteString;						//!< Notification string	
	const int notificationNo;							//!< Notification number
	Note(const std::string& note, const int& noteNo) : noteString(note), notificationNo(noteNo) {}
};

class Notification
{
	static int notificationCount;						//!< Total number of notifications created
	static std::list<Note> notificationList;			//!< List of notifications
	static std::mutex NListLock;						//!< Notification list lock
public:
/*******************************************************************************************
* @brief Return Notification
*
* @param[in] noteString			Notification string for which the notification is to be created			
* @return						Return the notification
*
* @details
* Assign notification number and insert the notification into the list.
********************************************************************************************/
	static const Note& newNotification(const std::string&);
/*******************************************************************************************
* @brief Return the notification number of the last notification generated
*
* @return						The last notification number
********************************************************************************************/
	static int lastNoteNumber();
/*******************************************************************************************
* @brief Create update notification record
*
* @param[in] writeBuffer		Buffer to which all notifications are to be written down
* @param[out] lastNoteNo		Notification number after which the update record is to be generated
*
* @details
* Update the lastNoteNo with the latest notification update number.
********************************************************************************************/
	static void createNoteRecord(std::string&, int&);
};
#endif