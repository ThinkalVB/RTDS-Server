#ifndef ADVANCED_DLL_H
#define ADVANCED_DLL_H

#include <mutex>
template<typename T1>
class DLLController
{
	T1* _begin = nullptr;
	T1* _end = nullptr;
	int _size = 0;
	std::mutex _dll_lock;
public:
	T1* begin();
	int size();
	template<typename T2> friend class DLLNode;
};

template<typename T2>
class DLLNode
{
	T2* _next = nullptr;
	T2* _previous = nullptr;
public:
	void addToDLL(T2*);
	void removeFromDLL(T2*);

	T2* next();
	T2* previous();
};

template<typename T2>
inline void DLLNode<T2>::addToDLL(T2* thisPtr)
{
	std::lock_guard<std::mutex> lock(thisPtr->dllController._dll_lock);
	if (thisPtr->dllController._begin == nullptr)
	{
		thisPtr->dllController._begin = thisPtr;
		thisPtr->dllController._end = thisPtr;
		_next = nullptr;
		_previous = nullptr;
	}
	else
	{
		thisPtr->dllController._end->dllNode._next = thisPtr;
		_previous = thisPtr->dllController._end;
		_next = nullptr;
		thisPtr->dllController._end = thisPtr;
	}
	thisPtr->dllController._size++;
}

template<typename T2>
inline void DLLNode<T2>::removeFromDLL(T2* thisPtr)
{
	thisPtr->dllController._size--;
	std::lock_guard<std::mutex> lock(thisPtr->dllController._dll_lock);

	if (_next == nullptr && _previous == nullptr)
	{
		thisPtr->dllController._begin = nullptr;
		thisPtr->dllController._end = nullptr;
	}
	else if (_next != nullptr && _previous != nullptr)
	{
		_previous->dllNode._next = _next;
		_next->dllNode._previous = _previous;
	}
	else if (thisPtr->dllController._begin == thisPtr)
	{
		thisPtr->dllController._begin = _next;
		thisPtr->dllController._begin->dllNode._previous = nullptr;
	}
	else if (thisPtr->dllController._end == thisPtr)
	{
		_previous->dllNode._next = nullptr;
		thisPtr->dllController._end = thisPtr;
	}
}


template<typename T2>
inline T2* DLLNode<T2>::next()
{
	return _next;
}

template<typename T2>
inline T2* DLLNode<T2>::previous()
{
	return _previous;
}

template<typename T1>
inline T1* DLLController<T1>::begin()
{
	return _begin;
}

template<typename T1>
inline int DLLController<T1>::size()
{
	return _size;
}

#endif