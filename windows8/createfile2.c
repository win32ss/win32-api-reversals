HANDLE CreateFile2(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, LPCREATEFILE2_EXTENDED_PARAMETERS pCreateExParams)
/*
  win32 - March 17 2022
  Tonight, I noticed UserBenchmark calling this function from Windows 8, even though I spoofed it to Windows 7. So this might be useful on those OSes too.
  
  I'm not sure what the point is of this function; the documentation page only lists changes specific to UWP, and a few extra flags which CreateFileW may be
  able to use on later OSes.
  
*/
{
   if(!pCreateExParams)
     return CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, 0, NULL);
	 
   return CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, pCreateExParams->lpSecurityAttributes, dwCreationDisposition, pCreateExParams->dwFileAttributes | pCreateExParams->dwFileFlags | pCreateExParams->dwSecurityQosFlags, hTemplateFile);

}