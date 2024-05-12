#ifndef _ENV_VAR
#define _ENV_VAR

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "Utils.h"

#define CMDEXTVERSION 2
#define DELIM TEXT('%')


/*
* @brief Return time in H:mm:ss.SS format.
*/
LPTSTR GetTimeString(VOID);


/*
* @brief Covert date into formated string
*
* @param [out] lpDate - output formated date
* @param [in] BufferSize - the size of lpDate
* @param [in] dt - the date to format
*/
INT FormatDate(TCHAR* lpDate, DWORD BufferSize, LPSYSTEMTIME dt);


/*
* @brief Return the current date as a string
*
*/
LPTSTR GetDateString(VOID);

/*
* @brief return the value of the given environment variable from the system/user table
*
* @param [in] varName - the variable to get
*/
LPTSTR GetEnvVar(LPCTSTR varName);

/*
* @brief return the value of the given env var from the special list of from the system/user table
*
* @param [in] varName - the variable to get
*/
LPCTSTR GetEnvVarOrSpecial(LPCTSTR varName);

/*
* @brief convert the given env variable to its value including DOS string manipulations
*
* @param [in] Src - the env variable to convert
* @param [out] Dest - the output buffer
* @param [in] DestEnd - pointer to the end of dest buffer
* @param [out] DestIncLen - amount written to Dest
*/
BOOL SubstituteVar(PCTSTR Src, PTCHAR Dest, PTCHAR DestEnd, size_t* DestIncLen);

/*
* @brief extract all the environment variables in the given string
*
* @param [in, out] lpFileName - the file path to transform
* @param [in] nMaxPathLength - the maximum size of lpFileName
*/
BOOL ConvertEnvVariables(LPTSTR lpFileName, DWORD nMaxPathLength);

#endif