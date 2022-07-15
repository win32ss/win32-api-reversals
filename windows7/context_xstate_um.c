/*
The following three functions were introduced in Windows 7 SP1 and are required by .NET Core 5.

They are tailored for cases where AVX support is unavailable and the OS does not have context switching
support for the expanded AVX/AVX512 registers; I plan on providing an emulator for these cases in the future,
provided that usage of AVX instructions goes mainstream.

These functions assume the usage of native contexts.
*/

BOOL WINAPI InitializeContext(PVOID Buffer, DWORD ContextFlags, PCONTEXT *Context, PDWORD ContextLength)
/*
  This function only exists because the size of the CONTEXT_XSTATE area can vary depending on the 
  type of CPU features available/enabled. There is no variability in a pre-XState/AVX context, so it simply
  initializes a regular context.
*/
{
	PCONTEXT ctxint;
	if(!Buffer || *ContextLength < sizeof(CONTEXT))
	{
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		*ContextLength = sizeof(CONTEXT);
		return FALSE;
	}
	
	Buffer = (PVOID)ctxint;
	
	RtlZeroMemory(ctxint, sizeof(CONTEXT));
	
	*Context = (PCONTEXT)Buffer;
	
	ctxint->ContextFlags = ContextFlags;
	
	return TRUE;
}

DWORD64 WINAPI GetEnabledXStateFeatures()
{
	DWORD64 XState = 0;
	
	XState = 3; // bits 0 and 1 represent X87 and SSE respectively, which all AMD64 CPUs support.
	       
	return XState;
}

BOOL WINAPI SetXStateFeaturesMask(PCONTEXT Context, DWORD64 FeatureMask)
// AMD64 version of the function. I don't see the need to modify it for other archs for the purpose of the extended kernel yet.
{
	if(!(Context->ContextFlags & CONTEXT_AMD64))
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	
	if(FeatureMask & 3)
		Context->ContextFlags |= CONTEXT_FLOATING_POINT;
	
	return TRUE;
}