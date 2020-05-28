#include "notification.h"
#include "log.h"
#include "cmd_interpreter.h"

int Notification::_notificationCount;
std::list<Note> Notification::_notificationList;
std::mutex Notification::_NListLock;

const Note& Notification::_newNote(const std::string& noteStr)
{
	Log::log(noteStr);
	std::lock_guard<std::mutex> lock(_NListLock);
	_notificationCount++;
	if (_notificationList.size() > MAX_NOTE_SIZE)
		_notificationList.pop_front();
	return _notificationList.emplace_back(noteStr, _notificationCount);
}

const Note& Notification::makeUpdateNote(const Entry* entry)
{
	std::string noteStr = "[$] ";
	//entry->printBrief(noteStr);
	noteStr += '\x1e';				//!< Record separator
	return _newNote(noteStr);
}

const Note& Notification::makeAddNote(const Entry* entry)
{
	std::string noteStr = "[+] ";
	//entry->printBrief(noteStr);
	noteStr += '\x1e';				//!< Record separator
	return _newNote(noteStr);
}

const Note& Notification::makeRemoveNote(const Entry* entry)
{
	std::string noteStr = "[+] ";
	//entry->printBrief(noteStr);
	noteStr += '\x1e';				//!< Record separator
	return _newNote(noteStr);
}

Note::Note(const std::string& note, const int noteNo)
{
	noteString = note;
	notificationNo = noteNo;
}
