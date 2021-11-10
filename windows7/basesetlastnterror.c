/* win32 - November 9 2021
 This has been in NT forever, but was only exported starting with Windows 7.
 And I think I have encountered applications that do call it, albeit they are rare. */

ULONG BaseSetLastNTError(NTSTATUS Status) // Export names are case-sensitive. And the standard is to type "Nt" instead of "NT". This is an exception.
{
  ULONG DosError;

  DosError = RtlNtStatusToDosError(Status);

  SetLastError(DosError);

  return DosError;
  
}
