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

#define MAX_LINE_SIZE 512

#define BAD_PATH TEXT("c:\\evilevilevil")

#define USER_AGREEMENT TEXT('y')

#define ELF_MAGIC 0x7F454c46

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

BOOL IsExecFile(LPCTSTR lpPath);

/**
* @brief check if the given file content is of an nt file
*
* @param [in] lpBuffer - the buffer to check
* @param [in] BufferLen - the length of the given buffer
*/
BOOL IsNTFile(LPCSTR lpBuffer, DWORD BufferLen);

/**
* @brief check if the given file content is of an OS2 file
*
* @param [in] lpBuffer - the buffer to check
* @param [in] BufferLen - the length of the given buffer
*/
BOOL IsOS2File(LPCSTR lpBuffer, DWORD BufferLen);

/**
* @brief check if the given file content is of an ELF file
*
* @param [in] lpBuffer - the buffer to check
* @param [in] BufferLen - the length of the given buffer
*/
BOOL IsElfFile(LPCSTR lpBuffer, DWORD BufferLen);

/**
* @brief check if the given file content is of an executable
*
* @param [in] lpBuffer - the buffer to check
* @param [in] BufferLen - the length of the given buffer
*/
BOOL IsDataExe(LPCSTR lpBuffer, DWORD BufferLen);

#endif // !PROJECT_PROJECT

