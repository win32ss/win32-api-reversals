DWORD WINAPI GetMaximumProcessorCount(WORD GroupNumber)
/*
  Some of this may seem a little odd, but I found when testing the functions on Vista systems that had
  hyperthreading disabled in BIOS, that the output was "switched around".

  GetMaximumProcessorCount seems like it should have used the variable ntoskrnl!KeNumberProcessors which is placed in the
  SYSTEM_BASIC_INFORMATION struct at member NumberOfProcessors (KeQueryMaximumProcessorCount returns KeNumberProcessors as well).

  GetActiveProcessorCount should have used the affinity mask at member ActiveProcessorsAffinityMask (ntoskrnl!KeActiveProcessors)

  But the resulting output was as follows (on a 6C/12T system with HT disabled): 
  GetMaximumProcessorCount returns 6 (logical processors)
  GetActiveProcessorCount returns 12 (logical processors)

  Not good. CPU-Z will not load with these results.

  The functions were swapped around, and provided satisfactory results for CPU-Z (and presumably other
  hardware verification software). And most importantly the results now reflect what
  you would get with the official functions, but with lower overhead.
*/
{
	DWORD MaximumProcessorCount;
	NTSTATUS Status;
	INT i;
	SYSTEM_BASIC_INFORMATION sysbasic;
	if(GroupNumber != 0 && GroupNumber != ALL_PROCESSOR_GROUPS)
		return 0;
	else
	{
		Status = NtQuerySystemInformation(SystemBasicInformation, &sysbasic, sizeof(SYSTEM_BASIC_INFORMATION), NULL);	
		if (Status < 0)
		{
			BaseSetLastNTError(Status);
			return 0;
		}
      
		MaximumProcessorCount = 0;

#ifdef _X86_
		for(i = 0; i < 32; i++) // Maximum of 32 processors on x86 Windows;
#else
        for(i = 0; i < 64; i++)
#endif
		if(sysbasic.ActiveProcessorsAffinityMask & 1 << i)
			++MaximumProcessorCount;

		return MaximumProcessorCount;
	}
}

DWORD WINAPI GetActiveProcessorCount(WORD GroupNumber)
{
	NTSTATUS Status;
	SYSTEM_BASIC_INFORMATION sysbasic;
	if(GroupNumber != 0 && GroupNumber != ALL_PROCESSOR_GROUPS)
	{
		return 0;
	}
	else
	{
		Status = NtQuerySystemInformation(SystemBasicInformation, &sysbasic, sizeof(SYSTEM_BASIC_INFORMATION), NULL);	
		if (Status < 0)
		{
			BaseSetLastNTError(Status);
			return 0;
		}
		return sysbasic.NumberOfProcessors;
	}

}