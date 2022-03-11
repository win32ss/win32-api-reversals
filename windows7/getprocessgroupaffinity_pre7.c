BOOL GetProcessGroupAffinity(HANDLE hProcess, PUSHORT GroupCount, PUSHORT GroupArray)
// Technically, all CPUs before Windows 7 were in group 0, so the function implementations are changed to reflect this reality.
{
   _PROCESS_INFORMATION ProcessInfo;
   if(!GroupCount)
   {
     SetLastError(ERROR_INSUFFICIENT_BUFFER);
	 return FALSE;
   }
   
   ProcessInfo = NtQueryInformationProcess(hProcess, ProcessBasicInformation, &ProcessInfo, sizeof(_PROCESS_INFORMATION), NULL);
   // Like GetProcessId(hProcess) but with fewer steps
   if(ProcessInfo < 0) 
   {
	 BaseSetLastNTError(ProcessInfo);
     return FALSE;
   }
	 
   *GroupCount = 1;
   
   GroupArray[0] = 1;
   
   return TRUE;
   
}