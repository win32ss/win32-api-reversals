void WINAPI GetCurrentThreadStackLimits(PULONG_PTR LowLimit, PULONG_PTR HighLimit)
// used by latest TBB runtime.
{
   *LowLimit = NtCurrentTeb()->DeallocationStack;
   *HighLimit = NtCurrentTeb()->NtTib.StackBase;
} 