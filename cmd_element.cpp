#include "cmd_element.h"

const std::size_t& CommandElement::size()
{
	return _size;
}

bool CommandElement::isEmpty()
{
	if (_size == 0)
		return true;
	else
		return false;
}

void CommandElement::reset_for_read()
{
	_index = 0;
}

void CommandElement::reset()
{
	_size = 0;
	_index = 0;
}

const std::string_view& CommandElement::pop_front()
{
	_size--;
	auto currIndex = _index;
	_index++;
	return element[currIndex];
}

void CommandElement::pop_front(const int count)
{
	_size = _size - count;
	_index = _index + count;
}

const std::string_view& CommandElement::peek()
{
	return element[_index];
}

const std::string_view& CommandElement::peek_next()
{
	return element[_index + 1];
}

void CommandElement::push_back(std::string_view elem)
{
	_size++;
	element[_index] = elem;
	_index++;
}