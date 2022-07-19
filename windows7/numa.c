BOOL WINAPI GetNumaProcesorNodeEx(PPROCESSOR_NUMBER Processor, PUSHORT NodeNumber)
{
  if(!Processor)
  {
  SetLastError(ERROR_INVALID_PARAMETER);
  return FALSE;
  }
  return GetNumaProcessorNode(Processor->Number, (PUCHAR)NodeNumber);
}

BOOL WINAPI GetNumaNodeProcessorMaskEx(UCHAR Node, PGROUP_AFFINITY ProcessorMask)
{
   if(!ProcessorMask)
   {
   SetLastError(ERROR_INVALID_PARAMETER);
   return FALSE;
   }	   
   return GetNumaNodeProcessorMask(Node, &ProcessorMask->Mask);
}
