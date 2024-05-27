#ifndef CONTENT_VALIDATION_H
#define CONTENT_VALIDATION_H
#include <Windows.h>
#include <stdint.h>

#define ELF_MAGIC 0x7F454c46

#define MAC_MAGICK32 0xfeedface
#define MAC_MAGICK64 0xfeedfacf

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
* @brief check if the given file content is of an Mach-O file
*
* @param [in] lpBuffer - the buffer to check
* @param [in] BufferLen - the length of the given buffer
*/
BOOL IsMacOFile(LPCSTR lpBuffer, DWORD BufferLen);


/**
* @brief check if the given file content is of an executable
*
* @param [in] lpBuffer - the buffer to check
* @param [in] BufferLen - the length of the given buffer
*/
BOOL IsDataExe(LPCSTR lpBuffer, DWORD BufferLen);


#endif