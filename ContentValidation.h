#ifndef CONTENT_VALIDATION_H
#define CONTENT_VALIDATION_H
#include <Windows.h>
#include <stdint.h>

#define ELF_MAGIC 0x7F454c46

#define MAC_MAGICK32 0xfeedface
#define MAC_MAGICK64 0xfeedfacf

typedef struct _OLD_DOS_HEADER {
    WORD   e_magic;                     // Magic number
    WORD   e_cblp;                      // Bytes on last page of file
    WORD   e_cp;                        // Pages in file
    WORD   e_crlc;                      // Relocations
    WORD   e_cparhdr;                   // Size of header in paragraphs
    WORD   e_minalloc;                  // Minimum extra paragraphs needed
    WORD   e_maxalloc;                  // Maximum extra paragraphs needed
    WORD   e_ss;                        // Initial (relative) SS value
    WORD   e_sp;                        // Initial SP value
    WORD   e_csum;                      // Checksum
    WORD   e_ip;                        // Initial IP value
    WORD   e_cs;                        // Initial (relative) CS value
    WORD   e_lfarlc;                    // File address of relocation table
    WORD   e_ovno;                      // Overlay number
} OLD_DOS_HEADER;

/**
* @brief check if the given file content is of an nt file
*
* @param [in] lpBuffer - the buffer to check
* @param [in] BufferLen - the length of the given buffer
* @param [in] IsExe - tell if the given nt file content(PE) is an executable and not for example a dll
*/
BOOL IsNTFile(LPCSTR lpBuffer, DWORD BufferLen, BOOL* IsExe);

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