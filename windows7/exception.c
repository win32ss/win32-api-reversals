void WINAPI RaiseFailFastException(PEXCEPTION_RECORD pExceptionRecord, PCONTEXT pContextRecord, DWORD dwFlags)
/*
    win32 - March 4 2021
	API function introduced with Windows 7. Well-documented, except for dwFlags = 2, which I'll call 
	FAIL_FAST_DO_NOT_RAISE_HARD_ERROR.
	
	It's the only thing that keeps even VS2022 compiler/linker components from running on vanilla Vista.
*/
{
  PEXCEPTION_RECORD p_ExceptionRecord; 
  NTSTATUS Status; 
  PULONG Response;
  _EXCEPTION_RECORD ExceptionRecord; 
  _CONTEXT ContextRecord; 
  PVOID retaddr; 

  p_ExceptionRecord = pExceptionRecord;
  if ( pExceptionRecord )
  {
    pExceptionRecord->ExceptionFlags |= EXCEPTION_NONCONTINUABLE;
    if ( (dwFlags & FAIL_FAST_GENERATE_EXCEPTION_ADDRESS) != 0 )
      pExceptionRecord->ExceptionAddress = retaddr;
  }
  else
  {
    memset(&ExceptionRecord, 0, sizeof(ExceptionRecord));
    p_ExceptionRecord = &ExceptionRecord;
    ExceptionRecord.ExceptionAddress = retaddr;
    ExceptionRecord.ExceptionCode = STATUS_FAIL_FAST_EXCEPTION;
    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
  }
  if ( !pContextRecord )
  {
    memset(&ContextRecord, 0, sizeof(ContextRecord));
    RtlCaptureContext(&ContextRecord);
  }                                                                                                                             
  if ( SignalStartWerSvc() < STATUS_SUCCESS || (Status = WaitForWerSvc(), Status < STATUS_SUCCESS) || Status == STATUS_TIMEOUT /*|| 0x7FFE02F0 & 1) == 0*/ || RtlReportException(NULL, NULL, 0x100) == STATUS_NOT_SUPPORTED)
  {                                                                                                                             // ^ Not exactly sure how to point to that area, 
    if ( (dwFlags & 2) == 0 )                                                                                                   // but if that is true, exception handling is basically turned off.
      NtRaiseHardError(p_ExceptionRecord->ExceptionCode | 0x10000000, 0, 0, 0, 1, Response);                                    // The RtlReportException() call does the same thing, and will return either that or STATUS_INVALID_PARAMETER.
    TerminateProcess((HANDLE)-1, p_ExceptionRecord->ExceptionCode);
  }
  else
  {
    NtRaiseException(p_ExceptionRecord, &ContextRecord, 0);
  }
}