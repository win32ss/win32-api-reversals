BOOL GetProcessGroupAffinity(HANDLE hProcess, PUSHORT GroupCount, PUSHORT GroupArray)
// Technically, all CPUs before Windows 7 were in group 0, so the function implementations are changed to reflect this reality.
{
   _PROCESS_INFORMATION ProcessInfo;
   NTSTATUS Status;
   *GroupCount = 1;
   if(!GroupCount)
   {
     SetLastError(ERROR_INSUFFICIENT_BUFFER);
	 return FALSE;
   }
   
   Status = NtQueryInformationProcess(hProcess, ProcessBasicInformation, &ProcessInfo, sizeof(_PROCESS_INFORMATION), NULL);
   // Like GetProcessId(hProcess) but with fewer steps
   if(Status < 0) 
   {
	 BaseSetLastNTError(Status);
     return FALSE;
   }
   
   GroupArray[0] = 1;
   
   return TRUE;
   
}

BOOL SetThreadGroupAffinity(HANDLE hThread, const GROUP_AFFINITY *GroupAffinity, PGROUP_AFFINITY PreviousGroupAffinity)
	// Well, process affinity is supposed to match thread group affinity at least in basic cases.
	// That is why I rely on process affinity for obtaining the previous group affinity.
{
		DWORD_PTR ProcessAffinityMask;
		DWORD_PTR SystemAffinityMask;
		DWORD Pid;
		HANDLE hProcess;
		
		if(PreviousGroupAffinity)
		{		
			Pid = GetProcessIdOfThread(hThread);
		    hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, Pid);
			if(!hProcess)
				return FALSE;

			GetProcessAffinityMask(hProcess, &ProcessAffinityMask, &SystemAffinityMask);
			PreviousGroupAffinity->Mask = ProcessAffinityMask;
			CloseHandle(hProcess);
		}

		return SetThreadAffinityMask(hThread, GroupAffinity->Mask);
}

BOOL GetThreadGroupAffinity(HANDLE hThread, PGROUP_AFFINITY GroupAffinity)
	// This is basically the same procedure as above, with regards to obtaining the "previous" affinity
{
		DWORD_PTR ProcessAffinityMask;
		DWORD_PTR SystemAffinityMask;
		DWORD Pid;
		HANDLE hProcess;
		
			Pid = GetProcessIdOfThread(hThread);
		    hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, Pid);
			if(!hProcess)
				return FALSE;

			if(!GetProcessAffinityMask(hProcess, &ProcessAffinityMask, &SystemAffinityMask))
				return FALSE;
			GroupAffinity->Mask = ProcessAffinityMask;
			GroupAffinity->Group = 0;
                        CloseHandle(hProcess);

		return TRUE;
}

BOOL GetThreadIdealProcessorEx(HANDLE hThread, PPROCESSOR_NUMBER lpIdealProcessor)
	// SetThreadIdealProcessor returns the "previous" or current IdealProcessor value.
{
		lpIdealProcessor->Number = SetThreadIdealProcessor(hThread, MAXIMUM_PROCESSORS);
		lpIdealProcessor->Group = 0;

		if(lpIdealProcessor->Number == -1)
			return FALSE;
		else
			return TRUE;
}
	
BOOL SetThreadIdealProcessorEx(HANDLE hThread, PPROCESSOR_NUMBER lpIdealProcessor, 
		PPROCESSOR_NUMBER lpPreviousIdealProcessor)
{
	lpPreviousIdealProcessor->Number = SetThreadIdealProcessor(hThread, lpIdealProcessor->Number);
        lpPreviousIdealProcessor->Group = 0;
	lpIdealProcessor->Group = 0;

	if(lpPreviousIdealProcessor->Number == -1)
		return FALSE;
	else
	return TRUE;
}
