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

        // Remove long paths/ symlink/junctions
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

    _tprintf_s(TEXT("what do you want to write:\n"));
    WCHAR UserLine[MAX_LINE_SIZE + 1] = { TEXT('\0') };
    DWORD amountRead = GetUserInput(UserLine, MAX_LINE_SIZE);
    if (!amountRead)
    {
        perror("failed to get uesr input");
        return FALSE;
    }
    
    if (IsDataExe(UserLine, amountRead))
    {
        _tprintf_s(TEXT("trying to write exe\n"));
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

    // Writes to the file
    // Handle - handle to the wanted file
    // UserLine - the line to write
    // _tcslen(UserLine) * sizeof(TCHAR) - the number of bytes to write
    // NULL - may return the amount of bytes written
    // NULL - used to asynchronies write to files
    if (!WriteFile(Handle, UserLine, amountRead * sizeof(TCHAR), NULL, NULL))
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

    if (IsExecFile(lpPath))
    {
        _tprintf_s(TEXT("invalid file format\n"));
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

DWORD GetExplorerOpenWithOfExtention(LPCTSTR Extension, LPTSTR lpOpenCommand, DWORD dwBufferSize)
{
    DWORD DataLength = dwBufferSize;

    TCHAR OpenReg[MAX_REG_NAME_LEN] = { 0 };
    _stprintf_s(OpenReg, MAX_REG_NAME_LEN, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\%s\\OpenWithList"), Extension);

    // Find What App index in optiones is used to open the given file
    DWORD Err = RegGetValue(HKEY_CURRENT_USER, OpenReg, TEXT("MRUList"), RRF_RT_REG_SZ, NULL, lpOpenCommand, &DataLength);
    if (ERROR_SUCCESS != Err)
    {
        return Err;
    }

    TCHAR Choice[2] = { 0 };
    Choice[0] = lpOpenCommand[0];
    DataLength = dwBufferSize;

    // find the name of the filed corespons with the option
    Err = RegGetValue(HKEY_CURRENT_USER, OpenReg, Choice, RRF_RT_REG_SZ, NULL, lpOpenCommand, &DataLength);
    if (ERROR_SUCCESS != Err)
    {
        return Err;
    }

    return ERROR_SUCCESS;
}

DWORD GetShellCommandOfExtention(LPCTSTR Extension, LPTSTR lpOpenCommand, DWORD dwBufferSize)
{
    TCHAR FileType[1024] = { 0 };
    DWORD DataLength = 1024;

    // Find What type of file coresponed with the given extention for example .bat, batfile
    DWORD Err = RegGetValue(HKEY_CLASSES_ROOT, Extension, NULL, RRF_RT_REG_SZ, NULL, &FileType, &DataLength);
    if (ERROR_SUCCESS != Err)
    {
        return Err;
    }

    TCHAR OpenReg[MAX_REG_NAME_LEN] = { 0 };
    _stprintf_s(OpenReg, MAX_REG_NAME_LEN, TEXT("%s\\shell\\open\\command"), FileType);

    LPTSTR tmp = malloc(sizeof(TCHAR) * dwBufferSize);
    if (tmp == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    DataLength = dwBufferSize;
    // Find What Command the shell uses
    Err = RegGetValue(HKEY_CLASSES_ROOT, OpenReg, NULL, RRF_RT_REG_SZ, NULL, tmp, &DataLength);
    if (ERROR_SUCCESS != Err)
    {
        free(tmp);
        return Err;
    }

    // Extruct the app it runs
    LPTSTR StartOfCommand = _tcschr(tmp, TEXT('"'));
    if (StartOfCommand == NULL)
    {
        _tcscpy_s(lpOpenCommand, dwBufferSize, tmp);
        free(tmp);
        return TRUE;
    }

    StartOfCommand++;
    LPTSTR EndOfCommand = _tcschr(StartOfCommand, TEXT('"'));
    if (EndOfCommand == NULL)
    {
        _tcscpy_s(lpOpenCommand, dwBufferSize, tmp);
        free(tmp);
        return TRUE;
    }

    _tcsncpy_s(lpOpenCommand, dwBufferSize, StartOfCommand, EndOfCommand - StartOfCommand);
    free(tmp);

    return ERROR_SUCCESS;
}

BOOL IsAppValid(LPCTSTR AppName, DWORD AppNameLen)
{
    for (size_t i = 0; i < sizeof(ValidApps) / sizeof(ValidApps[0]); i++)
    {
        if (AppNameLen != _tcslen(ValidApps[i]))
        {
            continue;
        }

        if (_tcscmp(AppName, ValidApps[i]) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL TestFileExtention(LPCTSTR lpExtention, LPBOOL IsExec)
{
    if (lpExtention == NULL || IsExec == NULL)
    {
        printf("invalid params\n");
        return FALSE;
    }

    UINT uRetVal = 0;

    TCHAR szTempFileName[MAX_PATH] = { 0 };
    TCHAR lpTempPathBuffer[MAX_PATH] = TEXT(".");
    //  Generates a temporary file name. 
    uRetVal = GetTempFileName(lpTempPathBuffer, // directory for tmp files
        TEXT("P1"),     // temp file name prefix 
        0,                // create unique name 
        szTempFileName);  // buffer for name 

    if (uRetVal == 0)
    {
        printf("failed to get tmpfile \n");
        return FALSE;
    }

    // The function automatically create the file so i need to delete it
    DeleteFile(szTempFileName);

    DWORD CurrentNameLen = _tcslen(szTempFileName);
    if (CurrentNameLen + _tcsclen(lpExtention) >= MAX_PATH)
    {
        printf("name to big\n");
        return FALSE;
    }

    // Add the relevant extention
    _tcsncpy_s(&szTempFileName[CurrentNameLen], MAX_PATH - CurrentNameLen, lpExtention, _tcsclen(lpExtention));

    //  Creates the new file to write to for the upper-case version.
    HANDLE hTempFile = CreateFile((LPTSTR)szTempFileName, // file name 
        GENERIC_WRITE,        // open for write 
        0,                    // do not share 
        NULL,                 // default security 
        CREATE_ALWAYS,        // overwrite existing
        FILE_ATTRIBUTE_NORMAL,// normal file 
        NULL);                // no template 

    // Write invalid code
    if (!WriteFile(hTempFile, NOT_CODE, sizeof(NOT_CODE), NULL, NULL))
    {
        printf("failed to write file\n");
        DeleteFile(szTempFileName);
        return FALSE;
    }

    CloseHandle(hTempFile);

    // recognize .com and .exe files
    DWORD binaryType;
    if (GetBinaryType(szTempFileName, &binaryType))
    {
        *IsExec = TRUE;
        return TRUE;
    }

    if (!RunFile(szTempFileName, IsExec))
    {
        printf("failed to run process\n");
        return FALSE;
    }

    DeleteFile(szTempFileName);

    return TRUE;
}

BOOL RunFile(LPCSTR lpFileName, LPBOOL IsExec)
{
    if (lpFileName == NULL || IsExec == NULL)
    {
        printf("invalid params\n");
        return FALSE;
    }

    // "Double click" the new file
    SHELLEXECUTEINFO info = { 0 };
    info.cbSize = sizeof(info);
    info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
    info.lpVerb = TEXT("open");
    info.lpFile = lpFileName;
    info.nShow = SW_NORMAL;

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (!ShellExecuteEx(&info))
    {
        PrintWindowsError(GetLastError());
        return FALSE;
    }

    // If it failed then double click would also fail
    if (info.hProcess == 0)
    {
        // Wait for the popup
        Sleep(300);

        KEYBDINPUT key_input = { 0 };
        key_input.wVk = VK_ESCAPE;
        INPUT escape;
        escape.type = INPUT_KEYBOARD;
        escape.ki = key_input;
        // close the popup
        SendInput(1, &escape, sizeof(INPUT));

        *IsExec = FALSE;
        return TRUE;
    }

    // Give the process a bit of time to crush
    Sleep(1000);
    DWORD exit_code;
    GetExitCodeProcess(info.hProcess, &exit_code);
    if (exit_code == STILL_ACTIVE)
    {
        *IsExec = FALSE;
        TerminateProcess(info.hProcess, 1);
    }
    else
    {
        *IsExec = TRUE;
    }

    CloseHandle(info.hProcess);
    return TRUE;
}

BOOL IsExecFile(LPCTSTR lpPath)
{
    if (lpPath == NULL)
    {
        return FALSE;
    }

    LPCTSTR lpExtentntion = PathFindExtension(lpPath);
    if (_tcslen(lpExtentntion) == 0)
    {
        return FALSE;
    }

    DWORD dwLengthToCheck;
    LPCTSTR EndOfExtention = _tcschr(lpExtentntion, TEXT(':'));
    if (EndOfExtention != NULL)
    {
        dwLengthToCheck = EndOfExtention - lpExtentntion;
    }
    else
    {
        dwLengthToCheck = _tcslen(lpExtentntion);
    }

    BOOL IsExec = TRUE;
    if (!TestFileExtention(lpExtentntion, &IsExec))
    {
        printf("failed to validate file extention\n");
        return TRUE;
    }

    return IsExec;
}


