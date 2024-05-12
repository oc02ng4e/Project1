
#include "Project1.h"

#include <Windows.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <strsafe.h>
#include <UserEnv.h>
#include <aclapi.h>
#include <Winnetwk.h>



int _tmain(int argc, wchar_t* argv[])
{

    HINSTANCE hinstLib;
    UNICODE_STRING FFilePath = { 0 };

    hinstLib = LoadLibrary(TEXT("ntdll.dll"));

    // If the handle is valid, try to get the function address.

    if (hinstLib != NULL)
    {
        RtlDosPathNameToNtPathName_U_WithStatus = (DOS_TO_NT)GetProcAddress(hinstLib, "RtlDosPathNameToNtPathName_U_WithStatus");

        // If the function address is valid, call the function.

        if (NULL == RtlDosPathNameToNtPathName_U_WithStatus)
        {
            _tprintf_s(TEXT("failed to load function\n"));
            return 0;
        }
    }
    else
    {
        _tprintf_s(TEXT("failed to load dll\n"));
        return 0;
    }

    TCHAR FilePath[MAX_SUPPORTED_PATH] = { TEXT('\0') };

    while (TRUE)
    {
        TCHAR FilePath[MAX_SUPPORTED_PATH] = { '\0' };

        if (!CreateDirectory(BAD_PATH, NULL))
        {
            DWORD dwError = GetLastError();
            if (dwError != ERROR_ALREADY_EXISTS)
            {
                PrintWindowsError(dwError);
                break;
            }
        }

        if (!GetFilePath(FilePath, MAX_SUPPORTED_PATH))
        {
            continue;
        }

        if (!CreateParentDirs(FilePath, MAX_SUPPORTED_PATH))
        {
            _tprintf_s(TEXT("falied to create parent dirs\n"));
            continue;
        }

        if (!WriteUserInputToFile(FilePath))
        {
            _tprintf_s(TEXT("failed to write user input to file\n"));
            continue;
        }

        if (!PrintFromFile(FilePath))
        {
            _tprintf_s(TEXT("failed to print data from file\n"));
            continue;
        }
    }
    
    FreeLibrary(hinstLib);
    
    return 0;
    
}
