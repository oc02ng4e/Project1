#include "Project1.h"

BOOL GetFilePath(LPTSTR lpFileName, DWORD dwLength)
{
    if (RtlDosPathNameToNtPathName_U_WithStatus == NULL)
    {
        _tprintf_s(TEXT("missing RtlDosPathNameToNtPathName_U_WithStatus function\n"));
        return FALSE;
    }

    if (lpFileName == NULL || dwLength <= 1)
    {
        _tprintf_s(TEXT("invalid params\n"));
        return FALSE;
    }

    TCHAR FormatedPath[MAX_SUPPORTED_PATH] = { 0 };
    LPTSTR PathToValidate;

    memset(lpFileName, dwLength * sizeof(TCHAR), 0);

    while (TRUE)
    {
        _tprintf_s(TEXT("enter file path\n"));
        if (!GetUserInput(lpFileName, dwLength))
        {
            perror("failed to get file path\n");
            return FALSE;
        }
        
        // Resolve the env variables in the path
        if (!ConvertEnvVariables(lpFileName, MAX_SUPPORTED_PATH))
        {
            _tprintf_s(TEXT("failed to convert env variables\n"));
            return FALSE;
        }

        UNICODE_STRING str = { 0 };
        // Find The full path and convert to nt path
        NTSTATUS status = (RtlDosPathNameToNtPathName_U_WithStatus)(lpFileName, &str, NULL, 0);
        if (status != 0)
        {
            _tprintf_s(TEXT("failed to get nt path with status: %d\n"), status);
            return FALSE;
        }

        DWORD dwFormattedLength = str.Length / sizeof(TCHAR);
        _tcscpy_s(FormatedPath, MAX_SUPPORTED_PATH, str.Buffer);
        
        RtlFreeUnicodeString(&str);

        // Remove long pathes/ symlink/juncitons
        if (!ResolveClosestPath(FormatedPath, dwFormattedLength, lpFileName, dwLength))
        {
            return FALSE;
        }

        if (PathIsNetworkPath(lpFileName))
        {
            // Find the local location of the network path
            if (!FormatNetworkPath(lpFileName, FormatedPath, dwLength))
            {
                return FALSE;
            }

            PathToValidate = FormatedPath;
        }
        else {
            if (_tcslwr_s(lpFileName, dwLength) != 0)
            {
                _tprintf_s(TEXT("failed to convert to lower\n"));
                return FALSE;
            }

            PathToValidate = lpFileName;
        }
        
        // Check if the path is not inside BAD_PATH
        if (!IsPathValid(PathToValidate))
        {
            _tprintf_s(TEXT("invalid path\n"));
            return FALSE;
        }

        BOOL IsExist;
        if (!IsFileExist(lpFileName, &IsExist))
        {
            return FALSE;
        }

        if (!IsExist)
        {
            break;
        }

        // Check if there is no hard link into BAD_PATH
        if (!ValidHardLink(PathToValidate))
        {
            _tprintf_s(TEXT("invalid path\n"));
            return FALSE;
        }

        // only if the user say y the file will get overwritten
        _tprintf_s(TEXT("do you wish to delete the existing file? [y/N]\n"));
        TCHAR Response;
        INT ErrorCode = _tscanf_s(TEXT(" %c"), &Response, 1);
        if (ErrorCode == EOF)
        {
            perror("failed to get response");
            return FALSE;
        }

        if (Response == USER_AGREEMENT)
        {
            break;
        }
    }

    return TRUE;
}

BOOL WriteUserInputToFile(LPCTSTR lpFilePath)
{
    if (lpFilePath == NULL)
    {
        _tprintf_s(TEXT("invalid params\n"));
        return FALSE;
    }

    // Create a new file
    // lpFilePath - path to the file to open
    // GENERIC_WRITE - access needed to the file in this case i only need to write to the file
    // 0 - the file cant be open until the handler is closed (other values can be used to allow sharing a file) 
    // NULL - security attribute
    // CREATE_ALWAYS - create the file again even if it is already existing
    // FILE_ATTRIBUTE_NORMAL - say it is a normal file (other value are FILE_ATTRIBUTE_HIDDEN for a hidden file)
    // NULL - can supply template for attributes of the given file
    HANDLE Handle = CreateFile(lpFilePath,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (Handle == INVALID_HANDLE_VALUE)
    {
        _tprintf_s(TEXT("failed to create file\n"));
        PrintWindowsError(GetLastError());
        return FALSE;
    }

    _tprintf_s(TEXT("what do you want to write:\n"));
    WCHAR UserLine[MAX_LINE_SIZE + 1] = { TEXT('\0') };

    if (!GetUserInput(UserLine, MAX_LINE_SIZE))
    {
        perror("failed to get uesr input");
        return FALSE;
    }
    
    // Writes to the file
    // Handle - handle to the wanted file
    // UserLine - the line to write
    // _tcslen(UserLine) * sizeof(TCHAR) - the number of bytes to write
    // NULL - may return the amount of bytes written
    // NULL - used to asynchronies write to files
    if (!WriteFile(Handle, UserLine, _tcslen(UserLine) * sizeof(TCHAR), NULL, NULL))
    {
        PrintWindowsError(GetLastError());
        return FALSE;
    }

    // Close the given handler
    if (!CloseHandle(Handle))
    {
        PrintWindowsError(GetLastError());
        return FALSE;
    }

    return TRUE;
}

BOOL PrintFromFile(LPCTSTR lpFilePath)
{
    if (lpFilePath == NULL)
    {
        _tprintf_s(TEXT("invalid params\n"));
        return FALSE;
    }

    // Create a new file
    // lpFilePath - path to the file to open
    // GENERIC_READ - access needed to the file in this case i only need to read from the file
    // 0 - the file cant be open until the handler is closed (other values can be used to allow sharing a file) 
    // NULL - security attribute - contain for example the owner
    // OPEN_EXISTING - open only if the file exists
    // FILE_ATTRIBUTE_NORMAL - say it is a normal file (other value are FILE_ATTRIBUTE_HIDDEN for a hidden file)
    // NULL - can supply template for attributes of the given file
    HANDLE Handle = CreateFile(lpFilePath,
        GENERIC_READ ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (Handle == INVALID_HANDLE_VALUE)
    {
        _tprintf(TEXT("failed to create file\n"));
        PrintWindowsError(GetLastError());
        return FALSE;
    }

    // the extra space makes sure the string ends   
    TCHAR FileLine[MAX_LINE_SIZE + 1] = { TEXT('\0') };

    // Read data from the given file
    // Handle - handler to the file
    // FileLine - output line
    // sizeof(FileLine) * sizeof(TCHAR) - size of the output line in bytes
    if (!ReadFile(Handle, FileLine, sizeof(FileLine), NULL, NULL))
    {
        PrintWindowsError(GetLastError());
        return FALSE;
    }

    FileLine[MAX_LINE_SIZE] = TEXT('\0');

    _tprintf_s(TEXT("%s\n"), FileLine);

    if (!CloseHandle(Handle))
    {
        PrintWindowsError(GetLastError());
        return FALSE;
    }
    return TRUE;
}

BOOL ValidHardLink(LPCTSTR lpPath)
{
    TCHAR NextName[MAX_SUPPORTED_PATH] = { TEXT('\0') };
    DWORD FileLength = MAX_SUPPORTED_PATH;

    // Create a handle for searching the next file names of the give file
    HANDLE SearchHandle = FindFirstFileNameW(lpPath, 0, &FileLength, NextName);
    if (SearchHandle == INVALID_HANDLE_VALUE)
    {
        PrintWindowsError(GetLastError());
        return FALSE;
    }

    TCHAR FormatedPath[MAX_SUPPORTED_PATH] = { TEXT('\0') };
    FileLength = MAX_SUPPORTED_PATH;

    // Find The Next file names
    while (FindNextFileNameW(SearchHandle, &FileLength, NextName))
    {
        // the path return in bad format so it needs a small fix
        DWORD dwFormattedLength = GetFullPathName(NextName, MAX_SUPPORTED_PATH, FormatedPath, NULL);
        if (_tcslwr_s(FormatedPath, MAX_SUPPORTED_PATH) != 0)
        {
            _tprintf_s(TEXT("failed to convert to lower\n"));
            FindClose(SearchHandle);
            return FALSE;
        }

        // Checks for each of the file names if it is valid
        if (!IsPathValid(FormatedPath))
        {
            FindClose(SearchHandle);
            return FALSE;
        }

        // reset the FileLength input param
        FileLength = MAX_SUPPORTED_PATH;
    }

    FindClose(SearchHandle);

    DWORD nLastError = GetLastError();
    if (nLastError == ERROR_HANDLE_EOF)
    {
        return TRUE;
    }

    PrintWindowsError(nLastError);
    return FALSE;
}

BOOL IsPathValid(LPCTSTR lpPath)
{
    if (lpPath == NULL)
    {
        _tprintf_s(TEXT("invalid params\n"));
        return FALSE;
    }

    HANDLE EvilHandle = CreateFile(BAD_PATH,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL);

    if (EvilHandle == INVALID_HANDLE_VALUE)
    {
        PrintWindowsError(GetLastError());
        return FALSE;
    }
    
    BY_HANDLE_FILE_INFORMATION EvilInfo = { 0 };
    if (!GetFileInformationByHandle(EvilHandle, &EvilInfo))
    {
        PrintWindowsError(GetLastError());
        CloseHandle(EvilHandle);

        return FALSE;
    }

    DWORD dwMaxLength = _tcslen(lpPath);
    TCHAR* TmpPath = (TCHAR*)malloc((dwMaxLength + 1) * sizeof(TCHAR));
    if (TmpPath == NULL)
    {
        return FALSE;
    }

    // iterate over lpPath from end to start
    LPTSTR CurrentSection = lpPath[dwMaxLength - 1];
    DWORD CurrentIndex = dwMaxLength;

    BOOL IsDrive = FALSE;
    // Iterate over the given path and checks if any of the directorys are BAD_PATH
    while (CurrentSection != NULL && !IsDrive)
    {
        _tcsncpy_s(TmpPath, dwMaxLength + 1, lpPath, CurrentIndex);

        DWORD dwLen = _tcslen(TmpPath);
        // Solve problems with driver when it will act as relative
        if (TmpPath[dwLen - 1] == TEXT(':'))
        {
            IsDrive = TRUE;
            TmpPath[dwLen] = TEXT('\\');
            dwLen++;
            TmpPath[dwLen] = TEXT('\0');
        }

        HANDLE CurrentHandle = CreateFile(TmpPath,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL);

        if (CurrentHandle == INVALID_HANDLE_VALUE)
        {
            DWORD dwLastError = GetLastError();
            if (!(dwLastError == ERROR_FILE_NOT_FOUND || dwLastError == ERROR_PATH_NOT_FOUND || dwLastError == ERROR_BAD_FORMAT))
            {
                PrintWindowsError(GetLastError());
                CloseHandle(EvilHandle);
                free(TmpPath);
                return FALSE;
            }
        }
        else
        {
            BY_HANDLE_FILE_INFORMATION CurrentInfo = { 0 };
            if (!GetFileInformationByHandle(CurrentHandle, &CurrentInfo))
            {
                PrintWindowsError(GetLastError());
                CloseHandle(CurrentHandle);
                CloseHandle(EvilHandle);
                free(TmpPath);
                return FALSE;
            }

            CloseHandle(CurrentHandle);
            
            // dwVolumeSerialNumber, nFileIndexHigh and nFileIndexLow together uniquely identify a file in a computer
            if (CurrentInfo.dwVolumeSerialNumber == EvilInfo.dwVolumeSerialNumber &&
                CurrentInfo.nFileIndexHigh == EvilInfo.nFileIndexHigh &&
                CurrentInfo.nFileIndexLow == EvilInfo.nFileIndexLow)
            {
                CloseHandle(EvilHandle);
                free(TmpPath);
                return FALSE;
            }
        }
        
        CurrentSection = _tcsrchr(TmpPath, TEXT('\\'));
        CurrentIndex = (CurrentSection - TmpPath);
    }

    free(TmpPath);
    return TRUE;
}
