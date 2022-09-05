/*
  I will implement this function by reading from KiProcessorBlock.
  For each struct, these are the values that will be used:
  RelationProcessorPackage: (I will have a struct per CPU package and divide KeNumberProcessors by the value of the PackageProcessorSet bitmask to determine the number of packages in a 
  Flags = 0;                 "all-CPU" situation of course)
  EfficiencyClass = 0;
  GroupCount = 1;
  GroupMask(GROUP_AFFINITY) is:
    Mask = KeActiveProcessors.Bitmap (KeQueryActiveProcessors);
	Group = 0;
  RelationProcessorCore is the same except for Flags. (as many of these structs as there are logical processors)
  Flags = LTP_PC_SMT if LogicalProcessorsPerCore > 1, else 0
  RelationNumaNode:
  NodeNumber = expressed by the index value of KeNumberNodes[] - one struct per node
  GroupCount = 1;
  GroupMask = same as above
  RelationCache:
  Use _CACHE_DESCRIPTOR struct to obtain the values; there are 5 structs allocated in the block, presumably for levels L1 to theoretical L5. I assume that CacheCount keeps track of those.
  One per cache level.
  RelationGroup:
  Very simple.
  
  To calculate the number of structs needed per logical processor, I think it will go like this:
  ProcessorPackage
  + ProcessorCore
  + NumaNode
  + Cache*CacheCount
  + Group
  = 5 to 7
  
  Actually for a !ProcessorNumber situation, I think you only need a limited amount of ProcessorPackages and NumaNodes (and one Group).
  So:
    # of actual CPU packages
   + # of logical processors
   + # of NUMA nodes
   + # of actual CPU packages * CacheCount
   + Group
   
   To avoid wasting CPU time, the necessary buffer size can be calculated in advance. Once the requirement is satisfied, the function can be executed.
   
   Other notes: 
   (PVOID)KeNumberNodes = &KeNumberProcessors + 1;
   (PVOID*)KiProcessorBlock = &KeNumberProcessors + 0x638;
   KiProcessorBlock->ParentNode = offset 0x37C0
   KiProcessorBlock->LogicalProcessorsPerCore = offset 0x63F 
   CACHE_DESCRIPTOR KiProcessorBlock->Cache = offset 0x3A38
   KiProcessorBlock->CacheCount = offset 0x3A74
   ParentNode->NodeNumber = offset 0x54 (could be less with Vista) 
   KiProcessorBlock->PackageProcessorSet = offset 0x3B08
  
*/

NTSTATUS NTAPI KeQueryLogicalProcessorRelationshipNew(PPROCESSOR_NUMBER ProcessorNumber, LOGICAL_PROCESSOR_RELATIONSHIP RelationshipType,
													  PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX Information, PULONG Length)
{
   NTSTATUS Status = STATUS_SUCCESS;
   KAFFINITY ProcessorMask;
   PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX Iterator;
   ULONG ProcessorsInPackage = 0;
   ULONG NumberOfPackages;
   ULONG RequiredLength = 0;
   ULONG i;
   PVOID* KeNumberNodesPtr;
   CCHAR KeNumberNodes;
   KPRCB* KiProcessorBlock;
   CACHE_DESCRIPTOR* CPUCache;
   CHAR CacheCount;
   KAFFINITY ProcessorPackageAffinity;
	
   if(!Length)
	 return STATUS_INVALID_PARAMETER; // Turns out the NVIDIA drivers can call this function with a NULL pointer for Length

   if(ProcessorNumber)
   {
		if(ProcessorNumber->Number > KeNumberProcessors - 1)
			return STATUS_INVALID_PARAMETER;
   }

   KeNumberNodesPtr = &KeNumberProcessors + 1;
   KeNumberNodes = (CCHAR)*KeNumberNodesPtr;
   
   KiProcessorBlock = &KeNumberProcessors + 0x638;

   ProcessorPackageAffinity = KiProcessorBlock[0].PackageProcessorSet;
  
  if(ProcessorNumber)
  {
	  CacheCount = KiProcessorBlock[ProcessorNumber->Number].CacheCount;
	  CPUCache = &KiProcessorBlock[ProcessorNumber->Number].Cache;


	  if(RelationshipType == RelationAll || RelationshipType == RelationProcessorPackage)
	    RequiredLength += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
	  if(RelationshipType == RelationAll || RelationshipType == RelationProcessorCore)
		RequiredLength += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);  
	  if(RelationshipType == RelationAll || RelationshipType == RelationNumaNode)
	    RequiredLength += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX); // To determine the NUMA node a processor belongs to, use KNODE* KiProcessorBlock->ParentNode
	  if(RelationshipType == RelationAll || RelationshipType == RelationGroup)   // ^ from there, you can grab ParentNode->NodeNumber
		RequiredLength += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);  
	  if(RelationshipType == RelationAll || RelationshipType == RelationCache)
		RequiredLength += CacheCount*sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
	
	//  RequiredLength = 4*sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) + KiProcessorBlock[ProcessorNumber]->CacheCount*sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
	  
	  if(RequiredLength > *Length)
	  {
		  *Length = RequiredLength;
		  return STATUS_INFO_LENGTH_MISMATCH;	  
	  }
	  
	  *Length = RequiredLength;
  }
  else
  {
	  CacheCount = KiProcessorBlock[0].CacheCount;
      CPUCache = &KiProcessorBlock[0].Cache;

	  for(i = 0; i < 64; i++)
	  {
		  if((ProcessorPackageAffinity >> i) & 1)
			  ++ProcessorsInPackage;
	  }
	  
	  NumberOfPackages = ProcessorsInPackage / KeNumberProcessors;
	  
	  if(RelationshipType == RelationAll || RelationshipType == RelationProcessorPackage)
	    RequiredLength += NumberOfPackages*sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
	  if(RelationshipType == RelationAll || RelationshipType == RelationProcessorCore)
		RequiredLength += KeNumberProcessors*sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);  
	  if(RelationshipType == RelationAll || RelationshipType == RelationNumaNode)
	    RequiredLength += KeNumberNodes*sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
	  if(RelationshipType == RelationAll || RelationshipType == RelationGroup)
		RequiredLength += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);  
	  if(RelationshipType == RelationAll || RelationshipType == RelationCache)
		RequiredLength += CacheCount*sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX); 
	
	//  RequiredLength = KeNumberProcessors*sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) + NumberOfPackages*sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) +
	//                   KeNumberNodes*sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX) + KiProcessorBlock[ProcessorNumber]->CacheCount*sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)
	//				   + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
	  
	  if(RequiredLength > *Length)
	  {
		  *Length = RequiredLength;
		  return STATUS_INFO_LENGTH_MISMATCH;	  
	  }
	  
	  *Length = RequiredLength;
  }
  
  ProcessorMask = KeQueryActiveProcessors();
  
  Iterator = Information;
  
  if(ProcessorNumber)
  {
	  if(RelationshipType == RelationAll || RelationshipType == RelationProcessorPackage)
	  {
		  Iterator->Relationship = RelationProcessorPackage;
		  Iterator->Size = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
		  Iterator->Processor.Flags = 0;
		  Iterator->Processor.EfficiencyClass = 0;
		  Iterator->Processor.GroupCount = 1;
		  Iterator->Processor.GroupMask->Group = 0;
		  Iterator->Processor.GroupMask->Mask = ProcessorMask;
	  Iterator++;
	  }
     if(RelationshipType == RelationAll || RelationshipType == RelationProcessorCore)
	  {
		  Iterator->Relationship = RelationProcessorPackage;
		  Iterator->Size = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
		  Iterator->Processor.Flags = KiProcessorBlock[ProcessorNumber->Number].LogicalProcessorsPerCore > 1;
		  Iterator->Processor.EfficiencyClass = 0;
		  Iterator->Processor.GroupCount = 1;
		  Iterator->Processor.GroupMask->Group = 0;
		  Iterator->Processor.GroupMask->Mask = ProcessorMask;
	  Iterator++;
	  }
      if(RelationshipType == RelationAll || RelationshipType == RelationNumaNode)
	  {
		  Iterator->Relationship = RelationNumaNode;
		  Iterator->Size = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
		  Iterator->NumaNode.NodeNumber = KiProcessorBlock[ProcessorNumber->Number].ParentNode->NodeNumber;
		  Iterator->NumaNode.GroupMask.Group = 0;
		  Iterator->NumaNode.GroupMask.Mask = ProcessorMask;
		  Iterator->NumaNode.GroupCount = 1;
	  Iterator++;
	  }
	  if(RelationshipType == RelationAll || RelationshipType == RelationCache)
	  {
		  for(i = 0; i < KiProcessorBlock[ProcessorNumber->Number].CacheCount; i++)
		  {
			  Iterator->Relationship = RelationCache;
			  Iterator->Size = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
			  Iterator->Cache.Level = KiProcessorBlock[ProcessorNumber->Number].Cache.Level;
			  Iterator->Cache.Associativity = KiProcessorBlock[ProcessorNumber->Number].Cache.Associativity;
			  Iterator->Cache.LineSize = KiProcessorBlock[ProcessorNumber->Number].Cache.LineSize;
			  Iterator->Cache.CacheSize = KiProcessorBlock[ProcessorNumber->Number].Cache.Size;
			  Iterator->Cache.Type = KiProcessorBlock[ProcessorNumber->Number].Cache.Type;
			  Iterator->Cache.GroupCount = 1;
			  Iterator->Cache.GroupMask.Group = 0;
			  Iterator->Cache.GroupMask.Mask = ProcessorMask;

			  Iterator++;
		  }
	  }
	  if(RelationshipType == RelationAll || RelationshipType == RelationGroup)
	  {
		  Iterator->Relationship = RelationGroup;
		  Iterator->Size = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
		  Iterator->Group.MaximumGroupCount = 1;
		  Iterator->Group.ActiveGroupCount = 1;
		  Iterator->Group.GroupInfo->ActiveProcessorCount = KeNumberProcessors;
		  Iterator->Group.GroupInfo->ActiveProcessorMask = ProcessorMask;
		  Iterator->Group.GroupInfo->MaximumProcessorCount = KeQueryMaximumProcessorCount();
	  }
  }
  else
  {
	  if(RelationshipType == RelationAll || RelationshipType == RelationProcessorPackage)
	  {
        for(i = 0; i < NumberOfPackages; i++)
		{
		  Iterator->Relationship = RelationProcessorPackage;
		  Iterator->Size = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
		  Iterator->Processor.Flags = 0;
		  Iterator->Processor.EfficiencyClass = 0;
		  Iterator->Processor.GroupCount = 1;
		  Iterator->Processor.GroupMask->Group = 0;
		  Iterator->Processor.GroupMask->Mask = ProcessorMask;
	      Iterator++;
		}
	  }
     if(RelationshipType == RelationAll || RelationshipType == RelationProcessorCore)
	  {
		  for(i = 0; i < KeNumberProcessors; i++)
		  {
		  Iterator->Relationship = RelationProcessorPackage;
		  Iterator->Size = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
		  Iterator->Processor.Flags = KiProcessorBlock[0].LogicalProcessorsPerCore > 1;
		  Iterator->Processor.EfficiencyClass = 0;
		  Iterator->Processor.GroupCount = 1;
		  Iterator->Processor.GroupMask->Group = 0;
		  Iterator->Processor.GroupMask->Mask = ProcessorMask;
	      Iterator++;
		  }
	  }
      if(RelationshipType == RelationAll || RelationshipType == RelationNumaNode)
	  {
		  for(i = 0; i < KeNumberNodes; i++)
		  {
		  Iterator->Relationship = RelationNumaNode;
		  Iterator->Size = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
		  Iterator->NumaNode.NodeNumber = KiProcessorBlock[0].ParentNode->NodeNumber;
		  Iterator->NumaNode.GroupMask.Group = 0;
		  Iterator->NumaNode.GroupMask.Mask = ProcessorMask;
		  Iterator->NumaNode.GroupCount = 1;
	      Iterator++;
		  }
	  }
	  if(RelationshipType == RelationAll || RelationshipType == RelationCache)
	  {
		  for(i = 0; i < KiProcessorBlock[0].CacheCount; i++)
		  {
			  Iterator->Relationship = RelationCache;
			  Iterator->Size = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
			  Iterator->Cache.Level = KiProcessorBlock[0].Cache.Level;
			  Iterator->Cache.Associativity = KiProcessorBlock[0].Cache.Associativity;
			  Iterator->Cache.LineSize = KiProcessorBlock[0].Cache.LineSize;
			  Iterator->Cache.CacheSize = KiProcessorBlock[0].Cache.Size;
			  Iterator->Cache.Type = KiProcessorBlock[0].Cache.Type;
			  Iterator->Cache.GroupCount = 1;
			  Iterator->Cache.GroupMask.Group = 0;
			  Iterator->Cache.GroupMask.Mask = ProcessorMask;

			  Iterator++;
		  }
	  }
	  if(RelationshipType == RelationAll || RelationshipType == RelationGroup)
	  {
		  Iterator->Relationship = RelationGroup;
		  Iterator->Size = sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX);
		  Iterator->Group.MaximumGroupCount = 1;
		  Iterator->Group.ActiveGroupCount = 1;
		  Iterator->Group.GroupInfo->ActiveProcessorCount = KeNumberProcessors;
		  Iterator->Group.GroupInfo->ActiveProcessorMask = ProcessorMask;
		  Iterator->Group.GroupInfo->MaximumProcessorCount = KeQueryMaximumProcessorCount();
	  }
  }

  return Status;
}
