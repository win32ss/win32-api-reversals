/* win32 - November 9 2021 
   Reversal of SetProcessDEPPolicy, Vista version. 
   Unsure if NtSetInformationProcess will succeed on XP < SP2 and 2003 < SP1, as they do not know what DEP is.
   However, 2003 SP1/SP2 do know what DEP is while not having that function. So it may work as intended
   on those platforms.

   On non-x86 platforms, SetProcessDEPPolicy is a stub that returns STATUS_NOT_SUPPORTED. It may be useful
   on the OS revisions that do not recognize DEP as well.
*/

#ifdef WX86 

BOOL WINAPI SetProcessDEPPolicy(DWORD dwFlags)
{
  DWORD dwFlagscpy;
  NTSTATUS Status;

  dwFlagscpy = dwFlags;
  if ( (dwFlags & 0xFFFFFFFC) != 0 ) //check that only bits 0 and 1 are set
  {
    BaseSetLastNTError(STATUS_INVALID_PARAMETER);
    return FALSE;
  }
  if ( (dwFlags & PROCESS_DEP_ENABLE) != 0 )
  {
    dwFlags = 9;
    if ( (dwFlagscpy & PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION) != 0 )
      dwFlags = 13;
  }
  else
  {
    if ( (dwFlags & PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION) != 0 )
    {
      BaseSetLastNTError(STATUS_INVALID_PARAMETER_MIX);
      return FALSE;
    }
    dwFlags = 2; // Set DEP to be disabled for the process
  }
  Status = NtSetInformationProcess((HANDLE)-1, ProcessExecuteFlags, (PVOID)&dwFlags,(ULONG) 4); //class 0x22
  if ( Status < 0 )
  {
    BaseSetLastNTError(Status);
    return FALSE;
  }
  return TRUE;
}

#else

BOOL WINAPI SetProcessDEPPolicy(DWORD dwFlags)
{
  BaseSetLastNTError(STATUS_NOT_SUPPORTED);
  return FALSE;
}

#endif
