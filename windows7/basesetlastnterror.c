/* win32 - November 9 2021
 This has been in NT forever, but was only exported starting with Windows 7.
 And I have encountered applications that do call it, albeit they are rare. */

ULONG BaseSetLastNtError(NTSTATUS Status)
{
  ULONG DosError;

  DosError = RtlNtStatusToDosError(Status);

  SetLastError(DosError);

  return DosError;
  
}