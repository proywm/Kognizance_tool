#include <iostream>

#include "symbol.hpp"

#define INITONCE(x)  __sync_lock_test_and_set(&x, 1)

int Symbol::isBfdInit = 0;

bool Symbol::Load(char* path)
{
	long storage_needed;
        long i;
	
	/*
	* Initializing BFD once 
	*/
	if (!INITONCE(isBfdInit)) {
		printf("Initialized\n");
		bfd_init();
	
		thisBfd = bfd_openr(path, 0);
	    	if (thisBfd == nullptr) {
        		return false;
		}

		/* oddly, this is required for it to work... */
		bfd_check_format(thisBfd, bfd_object);
		
		storage_needed = bfd_get_symtab_upper_bound (thisBfd);

        	if (storage_needed <= 0)
        		return false;   

	        asymbol **symbol_table = (asymbol **) malloc (storage_needed);
        	number_of_symbols = bfd_canonicalize_symtab (thisBfd, symbol_table);

	        if (number_of_symbols < 0)
        		return false;

	        for (i = 0; i < number_of_symbols; i++) {
			const char *name = bfd_asymbol_name(symbol_table[i]);
			const long value = bfd_asymbol_value(symbol_table[i]);
			symbolMap.insert(std::pair<long, std::string>(value, name));
			symbolSectionEnd.insert(std::pair<long, long>(value, (bfd_asymbol_base(symbol_table[i])+ bfd_get_section_size(symbol_table[i]->section)) ));
			std::cout << "Start IP: " << std::hex << value << " section " << bfd_asymbol_base(symbol_table[i]) << " section size: " << bfd_get_section_size(symbol_table[i]->section) <<" Func: " << name << " \n";
        	}
		free(symbol_table);

		bfd_close(thisBfd);
	}
	return true;
}

long Symbol::getStartAddress(long addr)
{
	auto itfound = symbolMap.lower_bound(addr);
	auto sectionfound = symbolSectionEnd.lower_bound(addr);

	if (itfound == symbolMap.begin())
		return addr;		// Found at the beginning
	else if (itfound == symbolMap.end()) 
		return -1;		// No entry found 
	else 
	{
		if(symbolMap.find(addr) != symbolMap.end())
		{
			if(sectionfound->second >= addr ) //within section boundary
				return itfound->first;	//start of the function
			else
				return -1;	//out of scope
		}
		itfound--;
		sectionfound--;
		if(sectionfound->second >= addr )
			return itfound->first;	//returning the previous entry
		else
			return -1;      //out of scope
	}
}

std::string Symbol::getFunctionName(long addr)
{
	auto itfound = symbolMap.lower_bound(addr);
        if (itfound == symbolMap.begin())
                return itfound->second;            // Found at the beginning
        else if (itfound == symbolMap.end()) 
                return "";              // No entry found 
        else
        {
                if(symbolMap.find(addr) != symbolMap.end())
                        return itfound->second;  //start of the function
                return itfound->second;  //returning the previous entry
        }

}
