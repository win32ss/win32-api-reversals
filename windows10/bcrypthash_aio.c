NTSTATUS BCryptHash(BCRYPT_ALG_HANDLE hAlgorithm, PUCHAR pbSecret, ULONG cbSecret, PUCHAR pbInput, 
                    ULONG cbInput, PUCHAR pbOutput, ULONG cbOutput)
{
   BCRYPT_HASH_HANDLE hHash;
   NTSTATUS Status;
   PUCHAR pbHashObject;
   ULONG cbHashObject;
   ULONG ResultLength;
   
   BCryptGetProperty(hAlgorithm, L"ObjectLength", NULL, 0, &ResultLength, 0);
   
   cbHashObject = ResultLength;
   
   pbHashObject = (PUCHAR)RtlAllocateHeap(NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap, 0, cbHashObject);
   
   if(!pbHashObject)
	   return STATUS_NO_MEMORY;
   
   Status = BCryptGetProperty(hAlgorithm, L"ObjectLength", pbHashObject, cbHashObject, &ResultLength, 0);
   
   if(Status < STATUS_SUCCESS)
	    goto ReturnAndDeallocate;

   Status = BCryptCreateHash(hAlgorithm, &hHash, pbHashObject, cbHashObject, pbSecret, cbSecret, 0);

   if(Status < STATUS_SUCCESS)
	   goto ReturnAndDeallocate;

   Status = BCryptHashData(hHash, pbInput, cbInput, 0);   
   
   if(Status < STATUS_SUCCESS)
	   goto DestroyAndReturn;
   
   Status = BCryptFinishHash(hHash, pbOutput, cbOutput, 0);   
   
   if(Status < STATUS_SUCCESS)
	   goto DestroyAndReturn;   
   
  DestroyAndReturn:
     BCryptDestroyHash(hHash);
  ReturnAndDeallocate:
     RtlFreeHeap(NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap, 0, pbHashObject);
     return Status;

}