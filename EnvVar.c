#include "EnvVar.h"

LPTSTR GetTimeString(VOID)
{
    TCHAR cTimeSeparator = TEXT(':');
    TCHAR cDecimalSeparator = TEXT('.');
    static TCHAR szTime[12];
    SYSTEMTIME t;
    GetLocalTime(&t);
    _stprintf_s(szTime, 12, _T("%2d%c%02d%c%02d%c%02d"),
        t.wHour, cTimeSeparator, t.wMinute, cTimeSeparator,
        t.wSecond, cDecimalSeparator, t.wMilliseconds / 10);
    return szTime;
}

INT FormatDate(TCHAR* lpDate, DWORD BufferSize, LPSYSTEMTIME dt)
{
    /* Format date */
    WORD wYear = dt->wYear;
    TCHAR cDateSeparator = TEXT('/');
    return _stprintf_s(lpDate, BufferSize, _T("%02d%c%02d%c%0*d"),
        dt->wDay, cDateSeparator, dt->wMonth,
        cDateSeparator, 4, wYear);
}

LPTSTR GetDateString(VOID)
{
    static TCHAR szDate[32];
    SYSTEMTIME t;
    GetLocalTime(&t);

    FormatDate(szDate, 32, &t);
    return szDate;
}

LPTSTR GetEnvVar(LPCTSTR varName)
{
    static LPTSTR ret = NULL;
    UINT size;

    free(ret);
    ret = NULL;
    size = GetEnvironmentVariable(varName, NULL, 0);
    if (size > 0)
    {
        ret = malloc((size + 1) * sizeof(TCHAR));
        if (ret != NULL)
        {
            GetEnvironmentVariable(varName, ret, size + 1);
        }
    }

    return ret;
}

LPCTSTR GetEnvVarOrSpecial(LPCTSTR varName)
{
    static TCHAR ret[MAX_SUPPORTED_PATH];

    LPTSTR var = GetEnvVar(varName);
    if (var)
        return var;

    /* %CD% */
    if (_tcsicmp(varName, _T("CD")) == 0)
    {
        GetCurrentDirectory(ARRAYSIZE(ret), ret);
        return ret;
    }
    /* %DATE% */
    else if (_tcsicmp(varName, _T("DATE")) == 0)
    {
        return GetDateString();
    }
    /* %TIME% */
    else if (_tcsicmp(varName, _T("TIME")) == 0)
    {
        return GetTimeString();
    }
    /* %RANDOM% */
    else if (_tcsicmp(varName, _T("RANDOM")) == 0)
    {
        /* Get random number */
        _itot_s(rand(), ret, MAX_SUPPORTED_PATH, 10);
        return ret;
    }
    /* %CMDCMDLINE% */
    else if (_tcsicmp(varName, _T("CMDCMDLINE")) == 0)
    {
        return GetCommandLine();
    }
    /* %CMDEXTVERSION% */
    else if (_tcsicmp(varName, _T("CMDEXTVERSION")) == 0)
    {
        /* Set Command Extensions version number to CMDEXTVERSION */
        _itot_s(CMDEXTVERSION, ret, MAX_SUPPORTED_PATH, 10);
        return ret;
    }
    /* %ERRORLEVEL% */
    else if (_tcsicmp(varName, _T("ERRORLEVEL")) == 0)
    {
        _itot_s(0, ret, MAX_SUPPORTED_PATH, 10);
        return ret;
    }
    /* %HIGHESTNUMANODENUMBER% */
    else if (_tcsicmp(varName, _T("HIGHESTNUMANODENUMBER")) == 0)
    {
        ULONG NumaNodeNumber = 0;
        GetNumaHighestNodeNumber(&NumaNodeNumber);
        _itot_s(NumaNodeNumber, ret, MAX_SUPPORTED_PATH, 10);
        return ret;
    }

    return NULL;
}

BOOL SubstituteVar(PCTSTR Src, PTCHAR Dest, PTCHAR DestEnd, size_t* DestIncLen)
{
#define APPEND(From, Length) \
do { \
    if (Dest + (Length) > DestEnd) \
        goto too_long; \
    memcpy(Dest, (From), (Length) * sizeof(TCHAR)); \
    Dest += (Length); \
} while (0)

    PCTSTR Var;
    PCTSTR Start, End, SubstStart;
    TCHAR EndChr;
    size_t VarLength;

    Start = Src;
    End = Dest;
    *DestIncLen = 0;

    if (*Src != DELIM)
    {
        return FALSE;
    }

    Src++;

    /* If we are already at the end of the string, fail the substitution */
    SubstStart = Src;
    if (!*Src || *Src == _T('\r') || *Src == _T('\n'))
    {
        goto bad_subst;
    }

    /* Find the end of the variable name. A colon (:) will usually
     * end the name and begin the optional modifier, but not if it
     * is immediately followed by the delimiter (%VAR:%). */
    SubstStart = Src;
    while (*Src && *Src != DELIM && !(*Src == _T(':') && Src[1] != DELIM))
    {
        ++Src;
    }

    /* If we are either at the end of the string, or the delimiter
     * has been repeated more than once, fail the substitution. */
    if (!*Src || Src == SubstStart)
    {
        goto bad_subst;
    }

    EndChr = *Src;
    *(PTSTR)Src = _T('\0');
    Var = GetEnvVarOrSpecial(SubstStart);
    *(PTSTR)Src++ = EndChr;
    if (Var == NULL)
    {
        goto bad_subst;
    }

    VarLength = _tcslen(Var);

    if (EndChr == DELIM)
    {
        /* %VAR% - use as-is */
        APPEND(Var, VarLength);
    }
    else if (*Src == _T('~'))
    {
        /* %VAR:~[start][,length]% - Substring.
         * Negative values are offsets from the end.
         */
        SSIZE_T Start = _tcstol(Src + 1, (PTSTR*)&Src, 0);
        SSIZE_T End = (SSIZE_T)VarLength;
        if (Start < 0)
        {
            Start += VarLength;
        }

        Start = min(max(Start, 0), VarLength);
        if (*Src == _T(','))
        {
            End = _tcstol(Src + 1, (PTSTR*)&Src, 0);
            End += (End < 0) ? VarLength : Start;
            End = min(max(End, Start), VarLength);
        }
        if (*Src++ != DELIM)
        {
            goto bad_subst;
        }

        APPEND(&Var[Start], End - Start);
    }
    else
    {
        /* %VAR:old=new%  - Replace all occurrences of old with new.
         * %VAR:*old=new% - Replace first occurrence only,
         *                  and remove everything before it.
         */
        PCTSTR Old, New;
        size_t OldLength, NewLength;
        BOOL Star = FALSE;
        size_t LastMatch = 0, i = 0;

        if (*Src == _T('*'))
        {
            Star = TRUE;
            Src++;
        }

        /* The string to replace may contain the delimiter */
        Src = _tcschr(Old = Src, _T('='));
        if (Src == NULL)
        {
            goto bad_subst;
        }

        OldLength = Src++ - Old;
        if (OldLength == 0)
        {
            goto bad_subst;
        }

        New = Src;
        Src = _tcschr(New, DELIM);

        if (Src == NULL)
        {
            goto bad_subst;
        }

        NewLength = Src++ - New;

        while (i < VarLength)
        {
            if (_tcsnicmp(&Var[i], Old, OldLength) == 0)
            {
                if (!Star)
                {
                    APPEND(&Var[LastMatch], i - LastMatch);
                }

                APPEND(New, NewLength);
                i += OldLength;
                LastMatch = i;
                if (Star)
                    break;
                continue;
            }
            i++;
        }

        APPEND(&Var[LastMatch], VarLength - LastMatch);
    }

success:
    *DestIncLen = (Dest - End);
    return TRUE;

bad_subst:
    Src = SubstStart;
    /* Only if no batch context active do we echo the delimiter */
    return FALSE;

too_long:
    _tprintf_s(TEXT("buffer to small\n"));
    return FALSE;

#undef APPEND
}

BOOL ConvertEnvVariables(LPTSTR lpFileName, DWORD nMaxPathLength)
{
    LPTSTR StartOfVar = _tcschr(lpFileName, DELIM);
    if (StartOfVar == NULL)
    {
        return TRUE;
    }

    StartOfVar++;

    LPTSTR EndOfVar = _tcschr(StartOfVar, DELIM);
    if (EndOfVar == NULL)
    {
        return FALSE;
    }

    DWORD dwPrevIndex = 0;
    LPTSTR TmpPath = malloc((nMaxPathLength) * sizeof(TCHAR));
    if (TmpPath == NULL)
    {
        _tprintf_s(TEXT("no memory\n"));
        return FALSE;
    }

    DWORD dwAmountWritten = 0;

    LPTSTR VariableName = malloc((nMaxPathLength) * sizeof(TCHAR));
    if (VariableName == NULL)
    {
        _tprintf_s(TEXT("no memory\n"));
        free(TmpPath);
        return FALSE;
    }

    LPTSTR VariableValue = malloc((nMaxPathLength) * sizeof(TCHAR));
    if (VariableValue == NULL)
    {
        _tprintf_s(TEXT("no memory\n"));
        free(TmpPath);
        free(VariableName);
        return FALSE;
    }

    DWORD dwRet, dwErr;

    // run until there is only one %
    while (EndOfVar != NULL)
    {
        // Copy the env variable in the format of %varName%
        _tcsncpy_s(VariableName, nMaxPathLength, StartOfVar - 1, (EndOfVar + 1) - (StartOfVar - 1));
        LPTSTR EndOfValue = &VariableValue[nMaxPathLength - 1];
        size_t DestLen = 0;

        // Get the value of the env variable
        if (!SubstituteVar(VariableName, VariableValue, EndOfValue, &DestLen))
        {
            if (nMaxPathLength < dwAmountWritten || (nMaxPathLength - dwAmountWritten) < (EndOfVar - &lpFileName[dwPrevIndex]))
            {
                _tprintf_s(TEXT("buffer to small\n"));
                free(VariableName);
                free(VariableValue);
                free(TmpPath);
                return FALSE;
            }

            // if the env variable dont exist copy its string add move the var pointers
            _tcsncpy_s(&TmpPath[dwAmountWritten], nMaxPathLength - dwAmountWritten, &lpFileName[dwPrevIndex], EndOfVar - &lpFileName[dwPrevIndex]);
            dwAmountWritten = dwAmountWritten + EndOfVar - &lpFileName[dwPrevIndex];
            dwPrevIndex = EndOfVar - lpFileName;
            StartOfVar = EndOfVar + 1;
            EndOfVar = _tcschr(StartOfVar, DELIM);

        }
        else
        {
            // add the extra data between the current var name and the previos one
            if (StartOfVar - &lpFileName[dwPrevIndex] > 1)
            {
                if (nMaxPathLength < dwAmountWritten || (nMaxPathLength - dwAmountWritten) < (StartOfVar - &lpFileName[dwPrevIndex]) - 1)
                {
                    _tprintf_s(TEXT("buffer to small\n"));
                    free(VariableName);
                    free(VariableValue);
                    free(TmpPath);
                    return FALSE;
                }

                _tcsncpy_s(&TmpPath[dwAmountWritten], nMaxPathLength - dwAmountWritten, lpFileName, (StartOfVar - &lpFileName[dwPrevIndex]) - 1);
                dwAmountWritten = dwAmountWritten + (StartOfVar - &lpFileName[dwPrevIndex]) - 1;

            }

            // add the new var and move the pointers
            _tcsncpy_s(&TmpPath[dwAmountWritten], nMaxPathLength - dwAmountWritten, VariableValue, DestLen);
            dwAmountWritten += DestLen;

            dwPrevIndex = (EndOfVar - lpFileName) + 1;
            StartOfVar = _tcschr(EndOfVar + 1, DELIM);
            if (StartOfVar == NULL)
            {
                break;
            }

            EndOfVar = _tcschr(StartOfVar, DELIM);
        }
    }

    if (dwPrevIndex != _tcsclen(lpFileName))
    {
        _tcscpy_s(&TmpPath[dwAmountWritten], nMaxPathLength - dwAmountWritten, &lpFileName[dwPrevIndex]);
    }

    _tcscpy_s(lpFileName, nMaxPathLength, TmpPath, dwAmountWritten);

    free(VariableName);
    free(VariableValue);
    free(TmpPath);

    return TRUE;
}

