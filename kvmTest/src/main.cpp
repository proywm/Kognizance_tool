//#include <Python.h>
#include <iostream>
//#include "tool.hpp"
#include "mytool.hpp"

int main(int argc, char *argv[])
{
	MyTool::Instance();
	std::cout << "Running main" << std::endl;
	return 0;
}
