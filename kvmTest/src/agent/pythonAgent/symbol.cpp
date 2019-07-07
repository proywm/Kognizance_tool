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
			//std::cout << "Start IP: " << value << " Func: " << name << " \n";
        	}
		free(symbol_table);

		bfd_close(thisBfd);
	}
	return true;
}

long Symbol::getStartAddress(long addr)
{
	auto itfound = symbolMap.lower_bound(addr);
	if (itfound == symbolMap.begin())
		return addr;		// Found at the beginning
	else if (itfound == symbolMap.end()) 
		return -1;		// No entry found 
	else 
	{
		if(symbolMap.find(addr) != symbolMap.end())
			return itfound->first;	//start of the function
		itfound--;
		return itfound->first;	//returning the previous entry
	}
}
