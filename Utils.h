#ifndef _UTILS
#define _UTILS
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#define MAX_MESSAGE_SIZE  128
#define MAX_SUPPORTED_PATH 32770 

/**
* @brief get input from the user
*
* @param [out] lpUserInput - buffer to put the input
* @param [in] dwLength - the length of the buffer
*
* @return the amount read or 0 if failed;
* @note expects error from GetLastError
*/
DWORD GetUserInput(LPTSTR lpUserInput, DWORD dwLength);

/**
* @brief print the given windows error
*
* @param [in] dwError - error code to print
*
* @note expects error from GetLastError
*/
VOID PrintWindowsError(DWORD dwError);

#endif