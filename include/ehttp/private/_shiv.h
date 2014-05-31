#ifndef EHTTP_SHIV_H
#define EHTTP_SHIV_H

/*
 * Internal header used to bridge really useful functions that sadly aren't
 * (yet?) available everywhere.
 * 
 * Most of this stuff is POSIX things Windows simply decided not to do or do
 * in a weird way for some reason, really.
 */

#ifdef _WIN32
	#include <string.h>
	
	#define strcasecmp _stricmp
#endif

#endif
