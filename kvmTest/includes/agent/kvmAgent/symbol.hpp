#ifndef _SYMBOL_HPP_
#define _SYMBOL_HPP_

#include <map>
#include <string>
#include <atomic>

#include <bfd.h>

/*
*class symbol
* An interface symbol
*
*/

class Symbol
{
	bfd *thisBfd = nullptr;
	long number_of_symbols;

	//map of address, symbol pair. to search by IP
	std::multimap<long, std::string> symbolMap;
	std::multimap<long, long> symbolSectionEnd;

	static int isBfdInit;
public:
	Symbol(){};
	~Symbol(){};

	//bool getFunction(uint64_t* start, uint64_t* end);
	bool Load(char* path);
	long getStartAddress(long addr);
	std::string getFunctionName(long addr);
};

#endif
