/* win32 - November 9 2021
   Windows Server 2003 SP1 introduced two new PSAPI functions to complement the 25 available as of Server 2003 RTM.
   This was the last meaningful update to PSAPI. Windows 7 introduced PSAPI_VERSION_2, which did not 
   involve any upgrades to the API, but moved the code into kernel32.dll and stuck "K32" onto each
   function's exported name. psapi.dll became a stub directing to the newly moved code.
   
   The following is reversals of 2003 SP1's new PSAPI functions: QueryWorkingSetEx and GetWsChangesEx. */

BOOL WINAPI QueryWorkingSetEx(HANDLE hProcess, PVOID pv, DWORD cb)
{
  NTSTATUS Status;

  Status = NtQueryVirtualMemory(hProcess, NULL, MemoryWorkingSetExList, pv, cb, NULL); // This class seems to have been introduced with XP/2003, 0x4.
  if ( Status >= 0 )
    return TRUE;
  SetLastError(RtlNtStatusToDosError(Status)); 

  return FALSE;
}

BOOL WINAPI GetWsChangesEx(
  HANDLE                         hProcess,
  PPSAPI_WS_WATCH_INFORMATION_EX lpWatchInfoEx,
  PDWORD                         cb
)

{
  NTSTATUS Status;

  Status = NtQueryInformationProcess(hProcess, ProcessWorkingSetWatchEx, lpWatchInfoEx, *cb, cb); //class is number 0x2A
  if ( Status >= 0 )                                                                              //Also new to XP/2003.
    return TRUE;
  RtlSetLastWin32Error(RtlNtStatusToDosError(Status));
  return FALSE;
}
