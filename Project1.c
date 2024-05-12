#include "Project1.h"

BOOL GetUserInput(LPTSTR lpUserInput, DWORD dwLength)
{
    if (lpUserInput == NULL || dwLength <= 1)
    {
        _tprintf(TEXT("invalid params\n"));
        return FALSE;
    }

    // get rid of \n at the start of the buffer
    do {
        if (!_fgetts(lpUserInput, dwLength, stdin))
        {
            perror("failed to get user input");
            return FALSE;
        }
    } while (lpUserInput[0] == TEXT('\n'));

    // _fgetts places \n\0 at the end
    DWORD UserInputLength = _tcslen(lpUserInput);
    if (lpUserInput[UserInputLength - 1] == TEXT('\n'))
    {
        lpUserInput[UserInputLength - 1] = TEXT('\0');
    }

    return TRUE;
}

BOOL IsFileExist(LPCTSTR lpFileName, LPBOOL IsExist)
{
    if (lpFileName == NULL || IsExist == NULL)
    {
        _tprintf_s(TEXT("invalid params\n"));
        return FALSE;
    }

    // Get information on a specific file, in this case if i use it to understand if the file exist or not
    // lpFileName - path to the wanted file
    DWORD dwFileAttribute = GetFileAttributes(lpFileName);
    if (dwFileAttribute == INVALID_FILE_ATTRIBUTES)
    {
        DWORD dwLastError = GetLastError();
        if (dwLastError == ERROR_FILE_NOT_FOUND || dwLastError == ERROR_PATH_NOT_FOUND)
        {
            *IsExist = FALSE;
            return TRUE;
        }
        else
        {
            // if i got a diffrent error i assume that the file exist
            PrintWindowsError(dwLastError);
            return FALSE;
        }
    }

    *IsExist = TRUE;
    return TRUE;
}

BOOL ResolveClosestPath(LPCTSTR lpFileName, DWORD dwOriginalPathLength, LPTSTR lpLongFileName, DWORD dwMaxLength)
{
    if (lpFileName == NULL || dwOriginalPathLength < 1 || lpLongFileName == NULL || dwMaxLength < 4 || dwOriginalPathLength > dwMaxLength)
    {
        _tprintf_s(TEXT("invalid params\n"));
        return FALSE;
    }

    memset(lpLongFileName, dwMaxLength * sizeof(TCHAR), 0);

    TCHAR* TmpPath = (TCHAR*)malloc((dwMaxLength + 1) * sizeof(TCHAR));
    if (TmpPath == NULL)
    {
        return FALSE;
    }

    // add an extra / so the file opend would be the original
    _tcsncpy_s(TmpPath, dwMaxLength, lpFileName, dwOriginalPathLength);
    TmpPath[dwOriginalPathLength] = TEXT('\\');
    TmpPath[dwOriginalPathLength+1] = TEXT('\0');

    DWORD CurrentIndex = dwOriginalPathLength;

    BOOL IsDrive = FALSE;
    while (CurrentIndex > 0)
    {
        memset(lpLongFileName, dwMaxLength * sizeof(TCHAR), 0);

        // Save the last index in case there are no more \\ so i can copy the rest of the str
        LPTSTR CurrentSection = _tcsrchr(TmpPath, TEXT('\\'));
        if (CurrentSection == NULL)
        {
            CurrentIndex = dwOriginalPathLength;
        }
        else
        {
            // i need the +1 to include \\ in the seciton
            CurrentIndex = CurrentSection - TmpPath;
        }

        _tcsncpy_s(TmpPath, dwMaxLength, lpFileName, CurrentIndex);

        DWORD dwLen = _tcslen(TmpPath);
        // Solve problems with driver when it will act as relateive
        if (TmpPath[dwLen - 1] == TEXT(':'))
        {
            IsDrive = TRUE;
            TmpPath[dwLen] = TEXT('\\');
            dwLen++;
            TmpPath[dwLen] = TEXT('\0');
        }

        HANDLE Handle = CreateFile(TmpPath,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL);

        if (Handle == INVALID_HANDLE_VALUE)
        {
            DWORD dwLastError = GetLastError();
            if (dwLastError == ERROR_FILE_NOT_FOUND || dwLastError == ERROR_PATH_NOT_FOUND)
            {
                if (IsDrive)
                {
                    _tprintf(TEXT("Drive dont exist\n"));
                    return FALSE;
                }

                continue;
            }
            else
            {
                PrintWindowsError(GetLastError());
                free(TmpPath);
                return FALSE;
            }
        }

        // Find the final location of the file/directory opend
        DWORD AmountWritten = GetFinalPathNameByHandle(Handle, lpLongFileName, MAX_SUPPORTED_PATH, FILE_NAME_NORMALIZED);
        if (!AmountWritten)
        {
            PrintWindowsError(GetLastError());
            CloseHandle(Handle);
            free(TmpPath);
            return FALSE;
        }
        
        if (IsDrive)
        {
            CurrentIndex++;
        }

        _tcsncpy_s(&lpLongFileName[AmountWritten], dwMaxLength - AmountWritten, &lpFileName[CurrentIndex], dwOriginalPathLength - CurrentIndex);

        CloseHandle(Handle);
        break;
    }

    free(TmpPath);
    return TRUE;
}

BOOL NetworkToLocalPath(LPTSTR lpFileName, DWORD dwMaxLen)
{
    if (lpFileName == NULL)
    {
        _tprintf_s(TEXT("invalid params\n"));
        return FALSE;
    }


    PSHARE_INFO_502  BufPtr;
    NET_API_STATUS   res = 0;
    TCHAR servername[MAX_SUPPORTED_PATH] = { 0 };
    TCHAR netname[MAX_SUPPORTED_PATH] = { 0 };
    
    // set index after the prefix//
    size_t start_index = 2;
    if (_tcsncmp(lpFileName, TEXT("\\\\\\?\\UNC\\"), _tcslen(TEXT("\\\\\\?\\UNC\\"))))
    {
        start_index = _tcslen(TEXT("\\\\\\?\\UNC\\")) - 1;
    }

    LPTSTR ServerSection = _tcschr(&lpFileName[start_index], TEXT('\\'));
    if (ServerSection == NULL)
    {
        _tprintf_s(TEXT("invalid params\n"));
        return FALSE;
    }

    // skip the // at the start
    _tcsncpy_s(servername, MAX_SUPPORTED_PATH, &lpFileName[start_index], ServerSection - &lpFileName[start_index]);
    // skip the /
    ServerSection++;

    LPTSTR NetnameSection = _tcschr(ServerSection, TEXT('\\'));
    if (NetnameSection == NULL)
    {
        _tcsncpy_s(netname, MAX_SUPPORTED_PATH, ServerSection, _tcslen(ServerSection));
    }
    else
    {
        _tcsncpy_s(netname, MAX_SUPPORTED_PATH, ServerSection, NetnameSection - ServerSection);
    }

    TCHAR* TmpPath = (TCHAR*)malloc(dwMaxLen * sizeof(TCHAR));
    if (TmpPath == NULL)
    {
        _tprintf_s(TEXT("no memory\n"));
        return FALSE;
    }

    // return information about shared resources
    // servername - where to look for the file
    // netname - the name of the shared folder
    // 502 - he amount of information to recieve in this case it include the path of the given information
    // BufPtr - out buffer to the PSHARE_INFO_502 struct
    if ((res = NetShareGetInfo(servername, netname, 502, (LPBYTE*)&BufPtr)) == ERROR_SUCCESS)
    {
        DWORD DestLen = _tcslen(BufPtr->shi502_path);
        if (DestLen > dwMaxLen)
        {
            _tprintf_s(TEXT("buffer to small"));
            free(TmpPath);
            return FALSE;
        }
        
        // remove the \\ at the end of the path if it exist(exist only in case of root drive
        _tcsncpy_s(TmpPath, dwMaxLen, BufPtr->shi502_path, DestLen);
        if (TmpPath[DestLen - 1] == TEXT('\\'))
        {
            DestLen--;
        }

        if (NetnameSection != NULL)
        {
            _tcsncpy_s(&TmpPath[DestLen], dwMaxLen - DestLen, NetnameSection, _tcslen(NetnameSection));
        }

        NetApiBufferFree(BufPtr);
    }
    else
    {
        _tprintf_s(TEXT("failed to get network information"));
        free(TmpPath);
        return FALSE;
    }

    _tcscpy_s(lpFileName, dwMaxLen, TmpPath);
    free(TmpPath);

    return TRUE;
}

BOOL FormatNetworkPath(LPTSTR lpFileName, LPTSTR lpServerPath, DWORD dwLength)
{
    if (lpFileName == NULL || dwLength <= 1)
    {
        _tprintf_s(TEXT("invalid params\n"));
        return FALSE;
    }

    memset(lpServerPath, 0, sizeof(TCHAR) * dwLength);
    _tcscpy_s(lpServerPath, dwLength, lpFileName);
        
    if (!NetworkToLocalPath(lpServerPath, dwLength))
    {
        _tprintf_s(TEXT("invalid network path\n"));
        return FALSE;
    }

    if (_tcslwr_s(lpFileName, dwLength) != 0)
    {
        _tprintf_s(TEXT("failed to convert to lower\n"));
        return FALSE;
    }

    if (_tcslwr_s(lpServerPath, dwLength) != 0)
    {
        _tprintf_s(TEXT("failed to convert to lower\n"));
        return FALSE;
    }

    return TRUE;
}

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
        
        if (!ConvertEnvVariables(lpFileName, MAX_SUPPORTED_PATH))
        {
            _tprintf_s(TEXT("failed to convert env variables\n"));
            return FALSE;
        }

        UNICODE_STRING str = { 0 };
        NTSTATUS status = (RtlDosPathNameToNtPathName_U_WithStatus)(lpFileName, &str, NULL, 0);
        if (status != 0)
        {
            _tprintf_s(TEXT("failed to get nt path with status: %d\n"), status);
            return FALSE;
        }

        DWORD dwFormattedLength = str.Length / sizeof(TCHAR);
        _tcscpy_s(FormatedPath, MAX_SUPPORTED_PATH, str.Buffer);
        
        RtlFreeUnicodeString(&str);
        if (!ResolveClosestPath(FormatedPath, dwFormattedLength, lpFileName, dwLength))
        {
            return FALSE;
        }

        if (PathIsNetworkPath(lpFileName))
        {
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

        if (!ValidHardLink(PathToValidate))
        {
            _tprintf_s(TEXT("invalid path\n"));
            return FALSE;
        }

        // only if the user say y the file will get overwriten
        _tprintf_s(TEXT("do you wish to deleate the existing file? [y/N]\n"));
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

LPTSTR GetParentDir(LPCTSTR lpPath, DWORD nMaxPathLength)
{
    if (lpPath == NULL || nMaxPathLength <= 1 )
    {
        _tprintf_s(TEXT("invalid params\n"));
        return NULL;
    }

    DWORD nPathLen = _tcslen(lpPath);

    LPCTSTR lpLastDelimeter = _tcsrchr(lpPath, TEXT('\\'));
    // no parent dir
    if (lpLastDelimeter == NULL)
    {
        return NULL;
    }

    // the diffrent in the pointer location is the length of the str
    DWORD nParentDirLength = ((lpLastDelimeter - lpPath) + 1);
    // empty parent dir
    if (nParentDirLength <= 1)
    {
        return NULL;
    }

    LPTSTR lpParentDir = (TCHAR*)malloc(nParentDirLength * sizeof(TCHAR));
    if (lpParentDir == NULL)
    {
        _tprintf_s(TEXT("no memory\n"));
        return NULL;
    }

    _tcsncpy_s(lpParentDir, nParentDirLength, lpPath, nParentDirLength - 1);
    lpParentDir[nParentDirLength - 1] = TEXT('\0');
    return lpParentDir;
}

BOOL CreateParentDirs(LPCTSTR lpPath, DWORD nMaxPathLength)
{
    if (lpPath == NULL || nMaxPathLength <= 1)
    {
        _tprintf_s(TEXT("invalid params\n"));
        return FALSE;
    }

    LPTSTR lpParentDir = GetParentDir(lpPath, nMaxPathLength);
    // there is no parent dir
    if (lpParentDir == NULL)
    {
        return TRUE;
    }

    BOOL RetVal = TRUE;
    // Try to create the parent dir
    // lpParentDir - path to the directory
    // NULL - security information for the file NULL sets it to default
    if (!CreateDirectory(lpParentDir, NULL))
    {
        // in case the parents dir parents dir dont exist create it first
        DWORD nLastError = GetLastError();
        if (nLastError == ERROR_PATH_NOT_FOUND)
        {
            RetVal = CreateParentDirs(lpParentDir, nMaxPathLength);
            if (RetVal)
            {
                if (!CreateDirectory(lpParentDir, NULL))
                {
                    PrintWindowsError(GetLastError());
                    RetVal = FALSE;
                }
            }
        }
        else if (nLastError == ERROR_ALREADY_EXISTS || nLastError == ERROR_ACCESS_DENIED)
        {
            RetVal = TRUE;
        }
        else
        {
            PrintWindowsError(GetLastError());
            RetVal = FALSE;
        }
    }

    free(lpParentDir);
    return RetVal;
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
    // NULL - used to asyncronyse write to files
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

    // Create a handle for searchin the next file names of the give file
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

        if (!IsPathValid(FormatedPath))
        {
            FindClose(SearchHandle);
            return FALSE;
        }
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
    while (CurrentSection != NULL && !IsDrive)
    {
        _tcsncpy_s(TmpPath, dwMaxLength + 1, lpPath, CurrentIndex);

        DWORD dwLen = _tcslen(TmpPath);
        // Solve problems with driver when it will act as relateive
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
            if (dwLastError == ERROR_FILE_NOT_FOUND || dwLastError == ERROR_PATH_NOT_FOUND || dwLastError == ERROR_BAD_FORMAT)
            {
                // continue the run in this case 
            }
            else
            {
                PrintWindowsError(GetLastError());
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

            // dwVolumeSerialNumber, nFileIndexHigh and nFileIndexLow together uniquely identify a file
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
