#ifndef _FILE_SYSTEM
#define _FILE_SYSTEM

#include <Windows.h>
#include <tchar.h>

#include "Utils.h" 

/**
* @brief Checks if a given file exist not
*
* @param [in] lpFileName - path to the file
* @param [out] IsExist - return if the file already exists
*
* @ Note if it fails to check it will assume that the file exist
*/
BOOL IsFileExist(LPCTSTR lpFileName, LPBOOL IsExist);

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


#endif // !_FILE_SYSTEM
