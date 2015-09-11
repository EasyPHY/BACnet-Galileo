/* ---------------------------------------------------------------------------------------

                EasyPHY(R) Reference BACnet Stack and Application 

        Copyright (C) 2015 ConnectEx, Inc.  All rights reserverd.

   All software deliverables are provided "AS IS" and ConnectEx, Inc. makes no
   representations or warranties, either express or implied, in respect to any of
   the foregoing, including without limitation, statuary or implied warranties
   or conditions of merchantablility, satisfactory quality and acceptance, fitness
   for a particular purpose or arising from a course of dealing or usage of
   trade, all of which are expressly disclaimed. The disclaimers and exclusions
   of this shall apply notwithstanding any failure of essential purpose of any
   limited remedy.

   License is granted for the use of this code solely for the development, test 
   and support of ConnectEx's EasyPHY(R) line of communication modules.
   Commercial use of this software only granted for hardware platforms that support
   ConnectEx's EasyPHY(R) line of communication modules.

   Contact us at info@connect-ex.com for further information.

 ------------------------------------------------------------------------------------------*/
 
#pragma once 

#include <net.h>

#ifdef _MSC_VER

#define LOG_INFO 1
#define LOG_ERR 2 
#define LOG_DAEMON 3
#define LOG_PID 4

#define openlog(a,b,c)
#define syslog(a,...) printf ( __VA_ARGS__ ) ; printf ( "\n" ) 
#define closelog()

/*
#define DEBUG_LEAK_DETECT
//*/
#define MEM_OVERFLOW_CHECK

#endif


#define DEBUG_LEVEL 3
#ifdef DEBUG_LEVEL
#ifdef _MSC_VER
#define PRINT(debug_level, a, ...) printf(a, __VA_ARGS__ )
#else
#define PRINT(debug_level, ...) printf(__VA_ARGS__)
// #define PRINT(debug_level, ...) if(debug_level <= DEBUG_LEVEL) fprintf(stderr, __VA_ARGS__)
// #define PRINT(debug_level, ...) syslog(LOG_INFO, __VA_ARGS__)
#endif
#else
#define PRINT(...)
#endif


#ifdef _MSC_VER
#define LockTransaction(mutexName) WaitForSingleObject( mutexName, INFINITE )
#define UnlockTransaction(mutexName) ReleaseMutex ( mutexName) 
#else
#define LockTransaction(mutexName) pthread_mutex_lock( &mutexName )
#define UnlockTransaction(mutexName) pthread_mutex_unlock( &mutexName )
#endif

void hexdump( char *title, void *mem, int size) ;
void ipdump(char *title, unsigned char *mem);

#define panic(desc)	printf ("PANIC: %s - %s, %d\n", desc, __FILE__, __LINE__ )

#define MyPortsTraffic  41797
#define MyPortsTerminal 48796
#define MyPortsPanic    502



