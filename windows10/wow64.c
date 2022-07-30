BOOL WINAPI IsWow64Process2(HANDLE hProcess, PUSHORT pProcessMachine, PUSHORT pNativeMachine)
/*
  An enhanced version of IsWow64Process() introduced with Windows 10 1511.
  Not only does it determine if the process is running under WOW64, but it also determines the
  WOW64 and native platforms.
*/
{
	BOOL Wow64Process;
	
	if(!pProcessMachine)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	
	if(!IsWow64Process(hProcess, &Wow64Process))
	{
		return FALSE;
	}
	
	if(!Wow64Process)
	{
		*pProcessMachine = IMAGE_FILE_MACHINE_UNKNOWN;
	}
	else
	{
		#ifdef _X86_ || _AMD64_ || _IA64_
		*pProcessMachine = IMAGE_FILE_MACHINE_I386;
		#elif _ARM64_ || _ARM_
		*pProcessMachine = IMAGE_FILE_MACHINE_ARM;
		#endif
		// No other Windows architecture has WOW64.
		
	}
	
    if(pNativeMachine)
    {
		#ifdef _X86_
		 *pNativeMachine = IMAGE_FILE_MACHINE_I386;
		#elif _AMD64_
		 *pNativeMachine = IMAGE_FILE_MACHINE_AMD64;
		#elif _ARM_
		 *pNativeMachine = IMAGE_FILE_MACHINE_ARM;
		#elif _ARM64_
		 *pNativeMachine = IMAGE_FILE_MACHINE_ARM64;
    	#endif
	}
	
	return TRUE;
}