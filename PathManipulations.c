#include "PathManipulations.h"

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

    // add an extra / so the file opened would be the original
    _tcsncpy_s(TmpPath, dwMaxLength, lpFileName, dwOriginalPathLength);
    TmpPath[dwOriginalPathLength] = TEXT('\\');
    TmpPath[dwOriginalPathLength + 1] = TEXT('\0');

    DWORD CurrentIndex = dwOriginalPathLength;

    // Because in order to open a drive for example c:/, there must be a / or it will look at it as relative
    BOOL IsDrive = FALSE;
    while (CurrentIndex > 0)
    {
        memset(lpLongFileName, dwMaxLength * sizeof(TCHAR), 0);

        // Save the last index in case there are no more \\ so i can copy the rest of the str
        LPTSTR CurrentSection = _tcsrchr(TmpPath, TEXT('\\'));
        if (CurrentSection == NULL)
        {
            // if there are no more section then i reached the start of the file name
            CurrentIndex = 0;
        }
        else
        {
            // the diffrent in the position is the end of the current section
            CurrentIndex = CurrentSection - TmpPath;
        }

        _tcsncpy_s(TmpPath, dwMaxLength, lpFileName, CurrentIndex);

        DWORD dwLen = _tcslen(TmpPath);
        // Solve problems with driver when it will act as relative
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
            // If the file donst exist keep shorthening the path 
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

        // Find the final location of the opened file/directory 
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
            // Skip the added /
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


    TCHAR servername[MAX_SUPPORTED_PATH] = { 0 };
    TCHAR netname[MAX_SUPPORTED_PATH] = { 0 };

    // set index after the prefix//
    size_t start_index = 2;
    if (_tcsncmp(lpFileName, TEXT("\\\\\\?\\UNC\\"), _tcslen(TEXT("\\\\\\?\\UNC\\"))))
    {
        start_index = _tcslen(TEXT("\\\\\\?\\UNC\\")) - 1;
    }

    // extrat the server name from the original path
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

    // extract the net name from the original path
    LPTSTR NetnameSection = _tcschr(ServerSection, TEXT('\\'));
    if (NetnameSection == NULL)
    {
        // if the entire path is server anem and netname
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

    PSHARE_INFO_502  BufPtr;
    NET_API_STATUS   res = 0;

    // return information about shared resources
    // servername - where to look for the file
    // netname - the name of the shared folder
    // 502 - he amount of information to receive in this case it include the path of the given information
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

        // remove the \\ at the end of the path if it exist(exist only in case of root drive)
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
        _tprintf_s(TEXT("failed to get network information\n"));
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
    
    // Find the local path of the network path 
    if (!NetworkToLocalPath(lpServerPath, dwLength))
    {
        _tprintf_s(TEXT("invalid network path\n"));
        return FALSE;
    }

    // convert to lower
    if (_tcslwr_s(lpFileName, dwLength) != 0)
    {
        _tprintf_s(TEXT("failed to convert to lower\n"));
        return FALSE;
    }

    // convert to lower
    if (_tcslwr_s(lpServerPath, dwLength) != 0)
    {
        _tprintf_s(TEXT("failed to convert to lower\n"));
        return FALSE;
    }

    return TRUE;
}
