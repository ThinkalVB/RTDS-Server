#include "notification.h"
#include "log.h"
#include "cmd_interpreter.h"

int Notification::notificationCount;
std::list<Note> Notification::notificationList;
std::mutex Notification::NListLock;

const Note& Notification::newNotification(const std::string& noteString)
{
	Log::log(noteString);
	std::lock_guard<std::mutex> lock(NListLock);
	notificationCount++;
	if (notificationList.size() > MAX_NOTE_SIZE)
		notificationList.pop_front();
	return notificationList.emplace_back(noteString, notificationCount);
}

int Notification::lastNoteNumber()
{
	std::lock_guard<std::mutex> lock(NListLock);
	return notificationCount;
}

void Notification::createNoteRecord(std::string& writeBuffer, int& lastNoteNo)
{
	for (auto noteItr = notificationList.begin(); noteItr != notificationList.end(); ++noteItr) 
	{
		if (noteItr->notificationNo > lastNoteNo)
		{
			writeBuffer += noteItr->noteString.substr(0, noteItr->noteString.size() - 1);
			writeBuffer += '\x1f';		//!< Unit separator
			lastNoteNo++;
		}
	}

	if (writeBuffer.size() == 0)
		writeBuffer += CmdInterpreter::RESP[(short)Response::NO_EXIST];
}