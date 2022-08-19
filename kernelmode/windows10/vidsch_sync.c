NTSTATUS VIDSCHAPI VidSchCreateSyncObject(VIDSCH_GLOBAL *pVidSchGlobal, DXGSYNCOBJECT* pDxgSyncObject, D3DDI_SYNCHRONIZATIONOBJECTINFO2* pObjectInfo,
                                          VIDSCH_CROSS_ADAPTER_SYNC_OBJECT_INFO *pCrossAdapterInfo, VIDMM_PAGING_QUEUE, *pPagingQueue,
								          VIDSCH_SYNC_OBJECT **ppVidSchSyncObject, VIDMM_MONITORED_FENCE_STORAGE *pMonitoredFenceStorage)
/*
  This is the backbone of the Direct3D kernel mode synchronization object initialization (second-generation). This is based on an early Windows 10 implementation.
*/
{
  ULONG NumberOfBytes;
  ULONG InitialFenceValue;
  NTSTATUS Status;
  VIDSCH_SYNC_OBJECT_CROSS_ADAPTER *vsoca;
  
  if(!pVidSchGlobal || !pDxgSyncObject || !pObjectInfo || !pCrossAdapterInfo || !pPagingQueue || !ppVidSchSyncObject || !pMonitoredFenceStorage)
	  return STATUS_INVALID_PARAMETER;

  *ppVidSchSyncObject = 0;
  if((pObjectInfo->Flags.Value & CrossAdapter) != 0) // WDDM 1.3 and up, bit 2
     NumberOfBytes = sizeof(VIDSCH_SYNC_OBJECT_CROSS_ADAPTER);
  else
     NumberOfBytes = sizeof(VIDSCH_SYNC_OBJECT);
	 
   vsoca = ExAllocatePoolWithTag(NonPagedPool, NumberOfBytes, "osiV"); // NonPagedPoolNx starting with Windows 8
   
   if(!vsoca)
	   return STATUS_NO_MEMORY;
   
   vsoca->pVidSchGlobal = pVidSchGlobal;
   vsoca->AllocTag = "osiV";
   vsoca->pSyncObject = pDxgSyncObject;
   vsoca->Reference = 0;
   vsoca->UnorderedWaiter = 0;
   vsoca->Sharable = pObjectInfo->Flags.Value & Shared;
   vsoca->DeferredWaits = (pObjectInfo->Flags.Value & D3DDDI_SYNCHRONIZATIONOBJECT_FLAGS_RESERVED0) != 0; // I guess this reserved flag can now be called DeferredWaits
   vsoca->CrossAdapter = (pObjectInfo->Flags.Value & CrossAdapter) != 0;	 
   if(pObjectInfo->Type == D3DDDI_SYNCHRONIZATION_MUTEX)
   {
	   vsoca->Type = MutexObject;
	   vsoca->Unorderable = FALSE;
	   if(pObjectInfo->SynchronizationMutex.InitialState)
	   {
		   vsoca->FenceData.InitialFenceValue = 0;
		   vsoca->FenceData.QueuedFenceValue = 0;
		   goto BuildLinkedList;
	   }
	   vsoca->FenceData.InitialFenceValue = -1;
	   vsoca->FenceData.QueuedFenceValue = -1;
	   goto BuildLinkedList;
   }
   if(Type == D3DDI_SEMAPHORE)
   {
	   if(pObjectInfo->Semaphore.InitialCount > pObjectInfo->SynchronizationMutex.InitialState)
	   {
		   ExFreePoolWithTag(vsoca, 0);
		   return STATUS_INVALID_PARAMETER;
	   }
	   vsoca->Type = SemaphoreObject;
	   vsoca->Unorderable = FALSE;
	   vsoca->SemaphoreData.Count = pObjectInfo->Semaphore.InitialCount;
	   vsoca->SemaphoreData.QueuedCount = pObjectInfo->Semaphore.InitialCount;
	   vsoca->SemaphoreData.MaxCount = pObjectInfo->Semaphore.MaxCount;
	   goto BuildLinkedList;
   }
   if(Type == D3DDDI_FENCE)
   {
	   vsoca->Unorderable = TRUE;
	   vsoca->Type = FenceObject;
	   vsoca->FenceData.CurrentFenceValue = pObjectInfo->Fence.FenceValue;
	   vsoca->FenceData.InitialFenceValue = pObjectInfo->Fence.FenceValue;
	   vsoca->FenceData.QueuedFenceValue = pObjectInfo->Fence.FenceValue;
	   goto BuildLinkedList;	   
   }
   if(Type != D3DDDI_CPU_NOTIFICATION)
   {
	   if(Type != D3DDDI_MONITORED_FENCE)
	   {
		   ExFreePoolWithTag(vsoca, 0);
		   return STATUS_INVALID_PARAMETER;		   
	   }
	   vsoca->Unorderable = TRUE;
	   vsoca->Type = MonitoredFenceObject;
	   if(pMonitoredFenceStorage)
	   {
			vsoca->FenceData.InitialFenceValue = pMonitoredFenceStorage->FenceStoragePage;
			vsoca->FenceData.QueuedFenceValue = pMonitoredFenceStorage->FenceKernelAddress;
			vsoca->FenceData.CurrentFenceValue = pMonitoredFenceStorage->OffsetInBytes;
			vsoca->MonitoredFenceData.QueuedFenceValue = pMonitoredFenceStorage->QueuedFenceValue;
	   }
	   else
	   {
		   Status = AllocateFenceStorageSlot(&vsoca->MonitoredFenceData, vsoca->Sharable != FALSE,
		                                     pDxgSyncObject == 0, pObjectInfo->Fence.FenceValue
											 pVidSchGlobal->DriverSupports64BitAtomics != 0);
			if(Status < STATUS_SUCCESS)
			{
			  ExFreePoolWithTag(vsoca, 0);
		      return Status;		
			}
	   }
	   vsoca->MonitoredFenceData.pPagingQueue = pMonitoredFenceStorage->pPagingQueue;
	   vsoca->Is64Bit = pDxgSyncObject->m_Is64Bit;		   
	   goto BuildLinkedList;  	  
   }
   	if(vsoca->Sharable)   
	{
		   ExFreePoolWithTag(vsoca, 0);
		   return STATUS_INVALID_PARAMETER;		
	}
	
	vsoca->Unorderable = FALSE;
	vsoca->Type = CpuNotificationObject;
	Status = ObReferenceObjectByHandle(pObjectInfo->Fence.FenceValue, 0x1F0003, *ExEventObjectType, 1, &InitialFenceValue, NULL);
	if(Status < STATUS_SUCCESS)
	{
		ExFreePoolWithTag(vsoca, 0);
		return Status;	
	}
	vsoca->FenceData.InitialFenceValue = InitialFenceValue;
	
BuildLinkedList:
    vsoca->QueuePacketWaitingListHead.Blink = &vsoca->QueuePacketWaitingListHead;
    vsoca->QueuePacketWaitingListHead.Flink = &vsoca->QueuePacketWaitingListHead;
    vsoca->QueuePacketUnorderedWaitingListHead.Blink = &vsoca->QueuePacketUnorderedWaitingListHead;
    vsoca->QueuePacketUnorderedWaitingListHead.Flink = &vsoca->QueuePacketUnorderedWaitingListHead;
  if ( vsoca->CrossAdapter )
  {
    vsoca->pCrossAdapterSyncObjectInfo = pCrossAdapterInfo;
    vsoca->pfnPostSignalCrossAdapter = VidSchPostSignalCrossAdapter;
    VidSchiAddSyncObjectToCrossAdapterInfo(vsoca);
  }
  *ppVidSchSyncObject = vsoca;
  return STATUS_SUCCESS;	
}