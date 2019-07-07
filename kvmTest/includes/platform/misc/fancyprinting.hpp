#ifndef _FANCYPRINTING_H
#define _FANCYPRINTING_H

#include <iostream>

#define MSGLOCATION		__FILE__ << ": "<< __FUNCTION__ << " : "

#define ERRMSG_BEGIN		"\033[1;31m" << MSGLOCATION
#define ERRMSG_END		"\033[0m"

#define SUCCESSMSG_BEGIN	"\033[1;32m" << MSGLOCATION
#define SUCCESSMSG_END        	"\033[0m"

#define TOOLMSG_BEGIN		"\033[1;36m" << MSGLOCATION
#define TOOLMSG_END		"\033[0m"

#define OUTMSG_BEGIN           "\033[1;30m"
#define OUTMSG_END             "\033[0m"

#define RESULTMSG_BEGIN		"\033[1;34m" << MSGLOCATION
#define RESULTMSG_END           "\033[0m"

#define PLATFORM_MSG_BEGIN	"\033[1;33m" << MSGLOCATION
#define PLATFORM_MSG_END	"\033[0m"

#define AGENT_MSG_BEGIN      "\033[1;35m" << MSGLOCATION
#define AGENT_MSG_END        "\033[0m"

#endif 
