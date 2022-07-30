	int WINAPI FindStringOrdinalS(DWORD dwFindStringOrdinalFlags, LPCWSTR lpStringSource, int cchSource, LPCWSTR lpStringValue,
                             int cchValue, BOOL bIgnoreCase)
{
	WCHAR* SourceCopy;
	WCHAR* ValueCopy;
	int i;
	int j;
	int k;
	int returnIndex;
	
	RtlSetLastWin32Error(ERROR_SUCCESS);
	
    if(!lpStringSource || !lpStringValue || cchSource < -1 || cchValue < -1)
	{
	   RtlSetLastWin32Error(ERROR_INVALID_PARAMETER);
	   return -1;
	}
	
	if((dwFindStringOrdinalFlags == FIND_FROMSTART &&
	   dwFindStringOrdinalFlags == FIND_FROMEND) ||
	   (dwFindStringOrdinalFlags == FIND_FROMSTART &&
	   dwFindStringOrdinalFlags == FIND_STARTSWITH) ||
	   (dwFindStringOrdinalFlags == FIND_FROMSTART &&
	   dwFindStringOrdinalFlags == FIND_ENDSWITH) ||
	   (dwFindStringOrdinalFlags == FIND_FROMEND &&
	   dwFindStringOrdinalFlags == FIND_STARTSWITH) ||
	   (dwFindStringOrdinalFlags == FIND_FROMEND &&
	   dwFindStringOrdinalFlags == FIND_ENDSWITH) ||
	   (dwFindStringOrdinalFlags == FIND_STARTSWITH &&
	   dwFindStringOrdinalFlags == FIND_ENDSWITH) ||
	    !bIgnoreCase && bIgnoreCase || 
		(dwFindStringOrdinalFlags & 0xFF0FFFFF) != 0) 
	   {
		   RtlSetLastWin32Error(ERROR_INVALID_FLAGS);
		   return -1;
	   }

	 if(cchSource == -1)
		 cchSource = wcslen(lpStringSource); // In the original function it is NlsStrLenW()
	 if(cchValue == -1)
		 cchValue = wcslen(lpStringValue);

	 
	 SourceCopy = (WCHAR*)RtlAllocateHeap(NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap, HEAP_ZERO_MEMORY, sizeof(WCHAR)*cchSource); // Possibly replace with RtlAllocateHeap()
	 ValueCopy = (WCHAR*)RtlAllocateHeap(NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap, HEAP_ZERO_MEMORY, sizeof(WCHAR)*cchValue);
	 
	 if(!SourceCopy || !ValueCopy)
	 {
		 RtlSetLastWin32Error(ERROR_NOT_ENOUGH_MEMORY);
		 return -1;
	 }

	 for(i = 0; i < cchSource; i++)
		 SourceCopy[i] = lpStringSource[i];

	 for(i = 0; i < cchValue; i++)
		 ValueCopy[i] = lpStringValue[i];

    if(bIgnoreCase) // This changes all upper case characters to lower case
	{
	  for(i = 0; i < cchSource; i++)
	  {
		  if(SourceCopy[i] >= 'A' && SourceCopy[i] <= 'Z')
		  SourceCopy[i] += 32;
	  }
	  for(i = 0; i < cchValue; i++)
	  {
		  if(ValueCopy[i] >= 'A' && ValueCopy[i] <= 'Z')
		  ValueCopy[i] += 32;
	  }
	  
	}

	returnIndex = -1;
	
	if(dwFindStringOrdinalFlags == FIND_FROMSTART)
	{
	   for(i = 0; i < cchSource; i++)
	   {
		   if (SourceCopy[i] == ValueCopy[0] && ((cchSource - i) >= cchValue))
		   {
			   returnIndex = i;
			   j = i;
			   for(k = 1; k < cchValue; k++)
			   {
				   ++j;
				   if(SourceCopy[j] != ValueCopy[k])
				   {
					   returnIndex = -1;
					   break;
				   }
			   }   
			   if(k == cchValue)
				   break;
		   }
	   }
	}
	
	if(dwFindStringOrdinalFlags == FIND_FROMEND)
	{
	   for(i = cchSource - 1; i >= 0; i--)
	   {
		   if (SourceCopy[i] == ValueCopy[0] && ((i + 1) >= cchValue))
		   {
			   returnIndex = i;
			   j = i;
			   for(k = 1; k < cchValue; k++)
			   {
				   --j;
				   if(SourceCopy[j] != ValueCopy[k])
				   {
					   returnIndex = -1;
					   break;
				   }
			   }
			   if(k == cchValue)
				   break;
		   }
	   }
	}
	
	if(dwFindStringOrdinalFlags == FIND_STARTSWITH)
	{
		if(SourceCopy[0] != ValueCopy[0])
			return -1;
		else
		{
			returnIndex = 0;
			
			for(i = 1; i < cchValue; i++)
			{
				if(SourceCopy[i] != ValueCopy[i])
				   {
					   returnIndex = -1;
					   break;
				   }
			}		
		}
	}
	
	if(dwFindStringOrdinalFlags == FIND_ENDSWITH)
	{
		if(SourceCopy[cchSource - cchValue] != ValueCopy[0])
			return -1;
		else
		{
			returnIndex = cchSource - cchValue;
			j = 0;
			
			for(i = cchSource - cchValue + 1; i < cchSource; i++)
			{
				++j;
				
				if(SourceCopy[i] != ValueCopy[j])
				   {
					   returnIndex = -1;
					   break;
				   }
			}
		}
	}
	
	RtlFreeHeap(NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap, 0, SourceCopy);
	RtlFreeHeap(NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap, 0, ValueCopy);
	
	return returnIndex;
}