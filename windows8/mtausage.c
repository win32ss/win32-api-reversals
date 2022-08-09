typedef struct _MTA_USAGE_INCREMENTOR
{
  ULONG ulTime;
  DWORD dwThreadId;
}MTA_USAGE_INCREMENTOR, *PMTA_USAGE_INCREMENTOR; // This struct came from the public symbol for combase.
// Very little information or none at all is stripped from the symbol, including TEB data.

void CreateMTAUsageThread()
{
	CoInitializeEx(NULL, 0);
	SuspendThread(GetCurrentThread());
	CoUninitialize();
}

HRESULT WINAPI CoIncrementMTAUsage(CO_MTA_USAGE_COOKIE *pCookie) 
/*
   My earlier drafts of this function did not create a thread, but used internal functions and incremented
   a global value in ole32.dll. Those are not suitable for wrappers.
   
   As the main intent of this function is to keep MTA alive when no MTA threads are running, the function
   creates a thread that initializes a MTA then suspends immediately. The thread is resumed by a calloc
   to CoDecrementMTAUsage.
*/
{
	PMTA_USAGE_INCREMENTOR MTAIncrementor;
	DWORD ThreadId;
	MTAIncrementor = CoTaskMemAlloc(sizeof(MTA_USAGE_INCREMENTOR));
	CreateThread(NULL, 0, CreateMTAUsageThread, 0, 0, &ThreadId);
	MTAIncrementor->dwThreadId = ThreadId;
	MTAIncrementor->ulTime = GetTickCount();

	*pCookie = MTAIncrementor;

	return S_OK;
}

HRESULT WINAPI CoDecrementMTAUsage(CO_MTA_USAGE_COOKIE *pCookie)
{
	HRESULT Result;
	if(pCookie)
	{
	PMTA_USAGE_INCREMENTOR MTAIncrementor = *pCookie;

	Result = ResumeThread(OpenThread(THREAD_SUSPEND_RESUME, FALSE, MTAIncrementor->dwThreadId));

	if(Result == -1)
		return E_FAIL;
	CoTaskMemFree(*pCookie);
	return S_OK;
	}
	else
		return E_INVALIDARG;
}