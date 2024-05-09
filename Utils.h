#ifndef _UTILS
#define _UTILS
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#define MAX_MESSAGE_SIZE  128
#define MAX_SUPPORTED_PATH 32770 


/**
* @brief print the given windows error
*
* @param [in] dwError - error code to print
*
* @note expects error from GetLastError
*/
VOID PrintWindowsError(DWORD dwError);
#endif