BOOL GetLogicalProcessorInformationEx(LOGICAL_PROCESSOR_RELATIONSHIP RelationshipType, PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX Buffer, PDWORD ReturnedLength)
/*
    A tough cookie of a function, which builds on the XP SP3/2003 SP1 function GetLogicalProcessorInformation. That function returns a struct with information 
	on either of the following:
	-Cache
	-NUMA nodes
	-Processor cores
	-Physical processor packages
	
	GetLogicalProcessorInformationEx does the same thing, but also returns information on processor groups and does not return all possible information if requested.
*/
{
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION BufferClassic;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Ptr;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX NewBuffer;
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX Temp;
	SYSTEM_INFO SysInfo;
	DWORD ReturnedLengthClassic;
	DWORD RequiredLength_All;
	DWORD RequiredLength;
	DWORD NewOffset;
	DWORD Offset;
#ifdef _X86_
	long ActiveProcessorMask;
#else
	__int64 ActiveProcessorMask;
#endif
	BOOL StructisCopied;
	BOOL BufferTooSmall = FALSE;
	
	if(!ReturnedLength || RelationshipType < 0 || (RelationshipType > 7 && RelationshipType != RelationAll))
	{
		RtlSetLastWin32Error(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
    if(Buffer == NULL)
	{
		RtlSetLastWin32Error(ERROR_NOACCESS);
		return FALSE;
	}
	
    BufferClassic = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)RtlAllocateHeap(NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap, 0, sizeof(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION));
	ReturnedLengthClassic = 0;
	while(!GetLogicalProcessorInformation(BufferClassic, &ReturnedLengthClassic))
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
		{
		RtlFreeHeap(NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap, 0, BufferClassic);
		BufferClassic = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)RtlAllocateHeap(NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap, 0, ReturnedLengthClassic);
		}
		else
		 return FALSE; // a lost cause; the first error is the only anticipated error. ERROR_NOACCESS is also possible, but shouldn't based on the way I've set it up.
		
		
	}
	
	GetSystemInfo(&SysInfo);
	
	ActiveProcessorMask = SysInfo.dwActiveProcessorMask;
	
	Ptr = BufferClassic;
	
	NewBuffer = Buffer;

	Offset = 0;
	RequiredLength_All = 0;
	RequiredLength = 0;
	NewOffset = 0;
 
	while (Offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= ReturnedLengthClassic)
	{
		StructisCopied = FALSE;
		switch(Ptr->Relationship)
		{
			case RelationNumaNode:
		     if(RelationshipType == RelationNumaNode || RelationshipType == RelationAll)
		     {
				 StructisCopied = TRUE;
				 Temp.Relationship = RelationNumaNode;
				 #ifdef _X86_
				 Temp.Size = 44;
				 RequiredLength = 44;
				 RequiredLength_All += 44;			 
			     #else
				 Temp.Size = 48;
				 RequiredLength = 48;
				 RequiredLength_All += 48;
				 #endif
				 Temp.NumaNode.NodeNumber = Ptr->NumaNode.NodeNumber;
				 Temp.NumaNode.GroupMask.Mask = (KAFFINITY) ActiveProcessorMask;
				 Temp.NumaNode.GroupMask.Group = 0;
				 if(RequiredLength_All > *ReturnedLength || BufferTooSmall)
				 BufferTooSmall = TRUE;
			     else
				 *NewBuffer = Temp;
			 }
			 break;
			 case RelationProcessorCore:
			 case RelationProcessorPackage:
			 if(RelationshipType == RelationProcessorCore || RelationshipType == RelationAll || RelationshipType == RelationProcessorPackage)
			 {
				 StructisCopied = TRUE;
				 if(Ptr->Relationship == RelationProcessorCore)
				 Temp.Relationship = RelationProcessorCore;
			     else
				 Temp.Relationship = RelationProcessorPackage;	
				 #ifdef _X86_
				 Temp.Size = 44;
				 RequiredLength = 44;
				 RequiredLength_All += 44;			 
			     #else
				 Temp.Size = 48;
				 RequiredLength = 48;
				 RequiredLength_All += 48;
				 #endif
				 Temp.Processor.Flags = Ptr->ProcessorCore.Flags;
				 Temp.Processor.EfficiencyClass = 0;
				 Temp.Processor.GroupCount = 1;
				 Temp.Processor.GroupMask->Mask =(KAFFINITY) ActiveProcessorMask;
				 Temp.Processor.GroupMask->Group = 0;
				 if(RequiredLength_All > *ReturnedLength || BufferTooSmall)
				 BufferTooSmall = TRUE;
			     else
				 *NewBuffer = Temp;
			 }
			 break;
			 case RelationCache:
			 if(RelationshipType == RelationCache || RelationshipType == RelationAll)
			 {
				 StructisCopied = TRUE;
				 Temp.Relationship = RelationCache;
				 Temp.Cache.Level = Ptr->Cache.Level;
				 #ifdef _X86_
				 Temp.Size = 52;
				 RequiredLength = 52;
				 RequiredLength_All += 52;		
				 #else
				 Temp.Size = 56;
				 RequiredLength = 56;
				 RequiredLength_All += 56;		
                 #endif				 
				 Temp.Cache.Associativity = Ptr->Cache.Associativity;
				 Temp.Cache.LineSize = Ptr->Cache.LineSize;
				 Temp.Cache.CacheSize = Ptr->Cache.Size;
				 Temp.Cache.Type = Ptr->Cache.Type;
				 Temp.Cache.GroupCount = 1;
				 Temp.Cache.GroupMask.Mask = (KAFFINITY) ActiveProcessorMask;
				 Temp.Cache.GroupMask.Group = 0;
				 if(RequiredLength_All > *ReturnedLength || BufferTooSmall)
				 BufferTooSmall = TRUE;
			     else
				 *NewBuffer = Temp;
				 
			 }
			 break;
			
		}
  
		Offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
		NewOffset = RequiredLength + Offset;
		RequiredLength = 0;   // to make sure it doesn't advance the SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX pointer
                              // if a SYSTEM_LOGICAL_PROCESSOR_INFORMATION struct contains irrelevant information
		if (StructisCopied && !BufferTooSmall)  
		NewBuffer++;
		Ptr++;
	}
	
	if(RelationshipType == RelationGroup || RelationshipType == RelationAll)
	{
		Temp.Relationship = RelationGroup;
		#ifdef _X86_
		Temp.Size = 76;
		RequiredLength = 76;
		RequiredLength_All += 76;			 
	    #else
		Temp.Size = 80;
	    RequiredLength = 80;
		RequiredLength_All += 80;
		#endif
		Temp.Group.MaximumGroupCount = 1;
		Temp.Group.ActiveGroupCount = 1;
		Temp.Group.GroupInfo->MaximumProcessorCount = SysInfo.dwNumberOfProcessors;
		Temp.Group.GroupInfo->ActiveProcessorCount = SysInfo.dwNumberOfProcessors;
		Temp.Group.GroupInfo->ActiveProcessorMask = (KAFFINITY) ActiveProcessorMask;
		if(RequiredLength_All > *ReturnedLength || BufferTooSmall)
		BufferTooSmall = TRUE;
	    else
		*NewBuffer = Temp;
	}
      RtlFreeHeap(NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap, 0, BufferClassic);
	  *ReturnedLength = RequiredLength_All;
	   if(BufferTooSmall)
	   {
		   RtlSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
		   return FALSE;
	   }
       
	   return TRUE;
	   

	
	
	}