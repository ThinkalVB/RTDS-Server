#ifndef CMD_ELEMENT_H
#define CMD_ELEMENT_H

#include <string_view>

/*******************************************************************************************
 * @brief Struct to hold individual command elements
 *
* @details
* This is an optimized container and shoudn't be used for any other purpose than populating elements.
* Before using pop_front(),push_back(), peek() and peek_next() explicit bound checks must be done.
* Populating the container must be done in a single stretch using push_back().
* After populating the container, call reset_for_read() to start reading.
* Before begining the push_back() streak call reset() once.
 ********************************************************************************************/
struct CommandElement
{
private:
	std::string_view element[5];
	std::size_t _size;
	std::size_t _index;
public:
	const std::size_t& size();
	bool isEmpty();
	void reset_for_read();
	void reset();

	const std::string_view& pop_front();
	void pop_front(int);
	const std::string_view& peek();
	const std::string_view& peek_next();
	void push_back(std::string_view);
};
#endif