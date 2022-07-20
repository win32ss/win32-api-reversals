HANDLE WINAPI FindFirstFileExW_Wrapper(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData,
								FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags)
/*
Windows 7 (and Windows 10 RS4) added some new enums/flags to both FINDEX_INFO_LEVELS and dwAdditionalFlags. 
FindFirstFileExW is designed to fail if flags not recognized by the function are used; for Vista and below, 
that caused problems with Chromium browsers when legacy fallback code was removed in June 2017.

The FindFirstFileExW calls used the new flags and enums, causing them to fail, resulting in severely hampered functionality:
extensions refused to install; temporary directories not being removed; cookies not being stored.

Luckily, some of the new flags can be ignored while others result in structs with less data than before. The only real change here
is to recognize the FindExInfoBasic level of FINDEX_INFO_LEVELS. The resulting WIN32_FIND_DATA struct is the same as with
FindExInfoStandard, but with the short file name removed.
*/
{
   HANDLE SearchHandle;

	if(dwAdditionalFlags & FIND_FIRST_EX_CASE_SENSITIVE)
		dwAdditionalFlags = FIND_FIRST_EX_CASE_SENSITIVE;
	else
		dwAdditionalFlags = 0;

	SearchHandle = FindFirstFileExW(lpFileName, FindExInfoStandard, lpFindFileData, fSearchOp, lpSearchFilter, dwAdditionalFlags);

    if(fInfoLevelId == FindExInfoBasic)
	{
		LPWIN32_FIND_DATAW Win32FileData = (LPWIN32_FIND_DATAW) lpFindFileData;
		Win32FileData->cAlternateFileName[0] = NULL;
	}

	return SearchHandle;

}