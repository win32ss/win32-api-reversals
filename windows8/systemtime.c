void WINAPI GetSystemTimePreciseAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
  LARGE_INTEGER Time;
  LARGE_INTEGER Time2;
 
  GetSystemTimeAsFileTime(lpSystemTimeAsFileTime);

  QueryPerformanceCounter(&Time);

  QueryPerformanceCounter(&Time2);

  lpSystemTimeAsFileTime->dwLowDateTime += (Time2.u.LowPart - Time.u.LowPart);
  lpSystemTimeAsFileTime->dwHighDateTime += (Time2.u.HighPart - Time.u.HighPart);
}