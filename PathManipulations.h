#ifndef _PATH_MANIPULATIONS_H
#define _PATH_MANIPULATIONS_H
#include <Windows.h>
#include <tchar.h>
#include <winternl.h>
#include <shlwapi.h>
#include <lm.h>

#include "Utils.h" 

// Define the RtlDosPathNameToNtPathName_U_WithStatus which is received form ntdll.dll in order to convert dos path to dos path
typedef NTSTATUS(__cdecl* DOS_TO_NT)(PCWSTR DosFileName, PUNICODE_STRING NtFileName, PWSTR* FilePart, PVOID Reserved);

DOS_TO_NT RtlDosPathNameToNtPathName_U_WithStatus;

/**
* @brief rfind the closet existing path in order to solve symlinks/junctions/short path name
*
* @param [in] lpFileName - original file name
* @param [in] dwOriginalPathLength - the length of the original path
* @param [out] lpLongFileName - the long file path
* @param [in] dwMaxLength - the max length of the buffer
*
* @ Note if it fails to check it will assume that the file exist
*/
BOOL ResolveClosestPath(LPCTSTR lpFileName, DWORD dwOriginalPathLength, LPTSTR lpLongFileName, DWORD dwMaxLength);

/**
* @brief find the destination of the given network path and rename the path
*
* @param [in, out] lpFileName - the network path to format
* @param [in] nLength - the maximum length lpFileName can hold
*
* @note it changes in place
*/
BOOL NetworkToLocalPath(LPTSTR lpFileName, DWORD dwMaxLen);

/**
* @brief format network path-
*
* @param [in, out] lpFileName - the file name to format
* @param [out] lpServerPath - what is the path of the resource on the server
* @param [in] nLength - length in THCAR of lpFileName
*
* @note it changes in place
*/
BOOL FormatNetworkPath(LPTSTR lpFileName, LPTSTR lpServerPath, DWORD dwLength);


#endif // !_PATH_MANIPULATIONS_H

