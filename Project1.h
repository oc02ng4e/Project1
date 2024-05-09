#ifndef PROJECT_PROJECT
#define PROJECT_PROJECT

#include <Windows.h>
#include <WinBase.h>

#include <shlwapi.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h> 
#include <lm.h>
#include <winternl.h>

#include "Utils.h"

#define MAX_LINE_SIZE 512

#define BAD_PATH TEXT("c:\\evilevilevil")

#define USER_AGREEMENT TEXT('y')


// Define the RtlDosPathNameToNtPathName_U_WithStatus which is recieved form ntdll.dll in order to convert dos path to dos path
typedef NTSTATUS(__cdecl* DOS_TO_NT)(PCWSTR DosFileName, PUNICODE_STRING NtFileName, PWSTR* FilePart, PVOID Reserved);

DOS_TO_NT RtlDosPathNameToNtPathName_U_WithStatus;

/**
* @brief get input from the user
*
* @param [out] lpUserInput - buffer to put the input
* @param [in] dwLength - the length of the buffer
* 
* @note expects error from GetLastError
*/
BOOL GetUserInput(LPTSTR lpUserInput, DWORD dwLength);

/**
* @brief Checks if a given file exist not
*
* @param [in] lpFileName - path to the file
* @param [out] IsExist - return if the file already exists
* 
* @ Note if it failes to check it will assume that the file exist
*/
BOOL IsFileExist(LPCTSTR lpFileName, LPBOOL IsExist);

/**
* @brief frsolve the existing path
*
* @param [in] lpFileName - original file name
* @param [in] dwOriginalPathLength - the length of the original path
* @param [out] lpLongFileName - the long file path
* @param [in] dwMaxLength - the max length of the buffer
* 
* @ Note if it failes to check it will assume that the file exist
*/
BOOL ResolveClosestPath(LPCTSTR lpFileName, DWORD dwOriginalPathLength, LPTSTR lpLongFileName, DWORD dwMaxLength);

/**
* @brief find the destination of the ggiven network path and rename the path
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

/**
* @brief Get from the user the desired file path and validate it
*
* @param [out] lpFileName - buffer to hold the file name
* @param [in] nLength - length in THCAR of lpFileName
*
* @note leave extra space in the buffer for \0
* @ return EOF on failure, specific error can be seen in errno
*/
BOOL GetFilePath(LPTSTR lpFileName, DWORD dwLength);


/**
* @brief Return the current file/dir parent directory
*
* @param [in] lpPath - path to the original file
*
* @warning the function malloc on the heap and the user need to free it
*/
LPTSTR GetParentDir(LPCTSTR lpPath, DWORD nMaxPathLength);


/**
* @brief create all the parent dirs of the given path
*
* @param [in] lpPath - path to the original file
*
* @param [in] nMaxPathLength - the maximum length of the path, should be as small as possible
*
*/
BOOL CreateParentDirs(LPCTSTR lpPath, DWORD nMaxPathLength);

/**
* @brief read user input, create new file and write the user input to it
*
* @param [in] lpFilePath - path to the file
*
* @note dont support writing \n to the file
*/
BOOL WriteUserInputToFile(LPCTSTR lpFilePath);

/**
* @brief read data from file and print it to the screen
*
* @param [in] lpFilePath - path to the file
*/
BOOL PrintFromFile(LPCTSTR lpFilePath);

/**
* @brief validate that all of the hard link to a given file are valid
*
* @param [in] lpPath - path to check
*/
BOOL ValidHardLink(LPCTSTR lpPath);

/**
* @brief validate that it ok to writre to the given path
*
* @param [in] lpPath - path to check
*/
BOOL IsPathValid(LPCTSTR lpPath);

#endif // !PROJECT_PROJECT

