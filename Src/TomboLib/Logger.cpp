#include <windows.h>
#include <tchar.h>
#include "Logger.h"

Logger g_Logger;

Logger::Logger() : bLogActive(FALSE)
{
}

BOOL Logger::Init(LPCTSTR pFile)
{
	BOOL bResult = fLogFile.Open(pFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, CREATE_ALWAYS);
	if (bResult) bLogActive = TRUE;
	return bResult;
}

BOOL Logger::WriteLog(LPCTSTR pStr)
{
	if (!bLogActive) return TRUE;
	fLogFile.Write((LPBYTE)pStr, _tcslen(pStr) * sizeof(TCHAR));
	return TRUE;
}

void Logger::Close()
{
	if (!bLogActive) return;
	fLogFile.Close();
}