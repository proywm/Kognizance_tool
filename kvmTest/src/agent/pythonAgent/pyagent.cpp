/**
* PyAgent interface implementations
*/
#include "pythonAgent/pyagent.hpp"

/*****************************************************************/

int pyAgent_is_enabled(void)
{
	return 0;
}

int pyAgent_print_stack(void)
{
//	PY_THREAD_STATE_T * tstate = NULL;
	sigprof_handler_template();
	return 0;
}
/**************************************************************/

