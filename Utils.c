#include "Utils.h"

VOID PrintWindowsError(DWORD dwError)
{
    TCHAR buffer[MAX_MESSAGE_SIZE] = { 0 };

    // Convert windows error into a proper error
    // FORMAT_MESSAGE_FROM_SYSTEM- convert error from the system table
    // NULL - location of message table
    // dwError - the id of the message in the table
    // 0 - set the language of the message
    // buffer - the output buffer
    // MAX_MESSAGE_SIZE - size of buffer in tchars
    // NULL - optional arguments
    if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, dwError, 0,
        buffer, MAX_MESSAGE_SIZE, NULL))
    {
        _tprintf_s(TEXT("Format message failed with 0x%x\n"), GetLastError());
        _tprintf_s(TEXT("original error code: 0x%x\n"), dwError);

        return;
    }

    _tprintf_s(TEXT("%s\n"), buffer);
}