#include "ContentValidation.h"


BOOL IsNTFile(LPCSTR lpBuffer, DWORD BufferLen)
{
    if (lpBuffer == NULL || BufferLen < sizeof(IMAGE_DOS_HEADER))
    {
        return FALSE;
    }

    IMAGE_DOS_HEADER* hdrDOS = (IMAGE_DOS_HEADER*)lpBuffer;
    if (BufferLen < hdrDOS->e_lfanew)
    {
        return FALSE;
    }

    // sizeof the IMAGE_DOS_HEADER, IMAGE_NT_HEADERS64 without the full size of the optional header + the size of the magic in the optional header
    if (BufferLen >= sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS64) - sizeof(IMAGE_OPTIONAL_HEADER32) + sizeof(WORD))
    {
        IMAGE_NT_HEADERS64* hdrNT = (IMAGE_NT_HEADERS64*)(lpBuffer + hdrDOS->e_lfanew);
        IMAGE_OPTIONAL_HEADER64 optHeader = hdrNT->OptionalHeader;
        IMAGE_FILE_HEADER fileHeader = hdrNT->FileHeader;

        // Checks the relevant magic, if the file is an executable and make sure it is not a dll
        if (hdrDOS->e_magic == IMAGE_DOS_SIGNATURE &&
            hdrNT->Signature == IMAGE_NT_SIGNATURE &&
            optHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC &&
            fileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE &&
            !(fileHeader.Characteristics & IMAGE_FILE_DLL))
        {
            return TRUE;
        }
    }

    // sizeof the IMAGE_DOS_HEADER, IMAGE_NT_HEADERS32 without the full size of the optional header + the size of the magic in the optional header
    if (BufferLen >= sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS32) - sizeof(IMAGE_OPTIONAL_HEADER32) + sizeof(WORD))
    {
        IMAGE_NT_HEADERS32* hdrNT = (IMAGE_NT_HEADERS32*)(lpBuffer + hdrDOS->e_lfanew);
        IMAGE_OPTIONAL_HEADER32 optHeader = hdrNT->OptionalHeader;
        IMAGE_FILE_HEADER fileHeader = hdrNT->FileHeader;

        // Checks the relevant magic, if the file is an executable and make sure it is not a dll
        if (hdrDOS->e_magic == IMAGE_DOS_SIGNATURE &&
            hdrNT->Signature == IMAGE_NT_SIGNATURE &&
            optHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC &&
            fileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE &&
            !(fileHeader.Characteristics & IMAGE_FILE_DLL))
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL IsOS2File(LPCSTR lpBuffer, DWORD BufferLen)
{
    if (lpBuffer == NULL || BufferLen < sizeof(IMAGE_DOS_HEADER))
    {
        return FALSE;
    }

    IMAGE_DOS_HEADER* hdrDOS = (IMAGE_DOS_HEADER*)lpBuffer;
    if (BufferLen < hdrDOS->e_lfanew)
    {
        return FALSE;
    }

    IMAGE_OS2_HEADER* hdrOS = (IMAGE_OS2_HEADER*)(lpBuffer + hdrDOS->e_lfanew);

    if (hdrDOS->e_magic == IMAGE_OS2_SIGNATURE ||
        hdrDOS->e_magic == IMAGE_OS2_SIGNATURE_LE)
    {
        return TRUE;
    }

    return FALSE;

}

BOOL IsElfFile(LPCSTR lpBuffer, DWORD BufferLen)
{
    if (lpBuffer == NULL || BufferLen < sizeof(ELF_MAGIC))
    {
        return FALSE;
    }

    // Check if the content starts with the elf magic either in big or little endian
    if (*(const uint32_t*)(lpBuffer) == ELF_MAGIC || *(const uint32_t*)(lpBuffer) == _byteswap_ulong(ELF_MAGIC))
    {
        return TRUE;
    }

    return FALSE;

}

BOOL IsMacOFile(LPCSTR lpBuffer, DWORD BufferLen)
{
    if (lpBuffer == NULL || BufferLen < sizeof(ELF_MAGIC))
    {
        return FALSE;
    }

    // Check if the content starts with the elf magic either in big or little endian
    if (*(const uint32_t*)(lpBuffer) == MAC_MAGICK32 ||
        *(const uint32_t*)(lpBuffer) == _byteswap_ulong(MAC_MAGICK32) ||
        *(const uint32_t*)(lpBuffer) == MAC_MAGICK64 ||
        *(const uint32_t*)(lpBuffer) == _byteswap_ulong(MAC_MAGICK64))
    {
        return TRUE;
    }

    return FALSE;

}

BOOL IsDataExe(LPCSTR lpBuffer, DWORD BufferLen)
{
    if (lpBuffer == NULL)
    {
        return FALSE;
    }

    if (BufferLen >= sizeof(IMAGE_DOS_HEADER))
    {
        IMAGE_DOS_HEADER* hdrDOS = (IMAGE_DOS_HEADER*)lpBuffer;

        // Dos executable
        if (hdrDOS->e_lfanew == 0 || hdrDOS->e_lfanew > BufferLen)
        {
            if (hdrDOS->e_magic == IMAGE_DOS_SIGNATURE || hdrDOS->e_magic == _byteswap_ushort(IMAGE_DOS_SIGNATURE))
            {
                return TRUE;
            }
        }

        if (IsNTFile(lpBuffer, BufferLen))
        {
            return TRUE;
        }

        if (IsOS2File(lpBuffer, BufferLen))
        {
            return TRUE;
        }
    }

    if (IsElfFile(lpBuffer, BufferLen))
    {
        return TRUE;
    }

    if (IsMacOFile(lpBuffer, BufferLen))
    {
        return TRUE;
    }

    return FALSE;
}

