#include "FileSystem.h"

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
            // if i got a different error i assume that the file exist
            PrintWindowsError(dwLastError);
            return FALSE;
        }
    }

    *IsExist = TRUE;
    return TRUE;
}

LPTSTR GetParentDir(LPCTSTR lpPath, DWORD nMaxPathLength)
{
    if (lpPath == NULL || nMaxPathLength <= 1)
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

    // the different in the pointer location is the length of the str
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
        // if the file already exist then there is no need to create, 
        // and if i get ERROR_ACCESS_DENIED then there is a chance that its already exist so i will try to continue the run
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