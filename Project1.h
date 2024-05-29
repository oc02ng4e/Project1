#ifndef PROJECT_PROJECT
#define PROJECT_PROJECT

#include <Windows.h>
#include <WinBase.h>

#include <stdio.h>
#include <string.h>
#include <tchar.h> 
#include <stdint.h>

#include "Utils.h"
#include "PathManipulations.h"
#include "FileSystem.h"
#include "ContentValidation.h"

#define MAX_LINE_SIZE 512

#define BAD_PATH TEXT("c:\\evilevilevil")

#define USER_AGREEMENT TEXT('y')

#define MAX_APP_NAME_LEN  1024

#define MAX_REG_NAME_LEN 2048

// List Of Apps Which is okey to use
static TCHAR ValidApps[][MAX_APP_NAME_LEN] = { TEXT("Microsoft.WindowsNotepad_8wekyb3d8bbwe!App"),
										TEXT("Microsoft.Windows.Photos_8wekyb3d8bbwe!App"),
										TEXT("devenv.exe"), 
										TEXT("Code.exe"),
										TEXT("notpad++.exe"),
										TEXT("WORDPAD.EXE")
};

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
* @brief validate that it ok to writer to the given path
*
* @param [in] lpPath - path to check
*/
BOOL IsPathValid(LPCTSTR lpPath);

/**
* @brief check the file extension and make sure it is not an extension of an executable
* 
* @param [in] lpPath - the path to check
*/
BOOL IsExecFile(LPCTSTR lpPath);

/**
* @brief Find the how the explorer would open a file with this extension
*
* @param [in] Extension - the extention to check
* @param [out] lpOpenCommand - the command that is used to open the given extention
* @param [in] dwBufferSize - the size of lpOpenCommand
*/
DWORD GetExplorerOpenWithOfExtention(LPCTSTR Extension, LPTSTR lpOpenCommand, DWORD dwBufferSize);

/**
* @brief Find the how the shell would open a file with this extension
*
* @param [in] Extension - the extention to check
* @param [out] lpOpenCommand - the command that is used to open the given extention
* @param [in] dwBufferSize - the size of lpOpenCommand
*/
DWORD GetShellCommandOfExtention(LPCTSTR Extension, LPTSTR lpOpenCommand, DWORD dwBufferSize);

/**
* @brief validate using a white list if the given app is valid to run
*
* @param [in] AppName - the app Name to check
* @param [in] AppNameLen - the size of AppName
*/
BOOL IsAppValid(LPCTSTR AppName, DWORD AppNameLen);

#endif // !PROJECT_PROJECT

