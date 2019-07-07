#include <iostream>

//#include "agent/pythonAgent/myagent.hpp"
#include "myagent.hpp"

/*
*
* Class MyAgent
* A class
*
*/

// Global static pointer used to ensure a single instance of the class.
MyAgent* MyAgent::a_pInstance = NULL;

/** This function is called to create an instance of the class. 
    Calling the constructor publicly is not allowed. The constructor 
    is private and is only called by this Instance function.
*/

MyAgent* MyAgent::Instance()
{
   if (!a_pInstance)   // Only allow one instance of class to be generated.
      a_pInstance = new MyAgent;

   return a_pInstance;
}

void MyAgent::agent_getCallchain()
{
	std::cout << "NativeAgent: agent_getCallchain" << std::endl;
}

