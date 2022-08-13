/*-
 * Copyright (c) 2008 Damien Bergamini <damien.bergamini@free.fr>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
 
 /*
  * This code has been adapted for the Windows Vista Extended Kernel. It is based off OpenBSD's implementation of PBKDF2.
  */
  
 #define SIZE_MAX 0xFFFFFFFF


NTSTATUS BCryptDeriveKeyPBKDF2(BCRYPT_ALG_HANDLE hPrf, const char *pbPassword, size_t cbPassword, const uint8_t *pbSalt,
    size_t cbSalt, uint8_t *pbDerivedKey, size_t cbDerivedKey, ULONGLONG cIterations)
{
	UCHAR* aSalt, *obuf; // Update this, use variable digest length obtained from BCryptGetProperty call
	UCHAR* d1, *d2;
	ULONG i, j;
	ULONG count, ResultLength;
	size_t r;
	DWORD DigestLength;
	BCRYPT_HASH_HANDLE hHash;
	NTSTATUS Status = STATUS_SUCCESS;

	if (cIterations < 1 || cbDerivedKey == 0 || !hPrf)
		return STATUS_INVALID_PARAMETER;
	if (cbSalt == 0 || cbSalt > SIZE_MAX - 4)
		return STATUS_INVALID_PARAMETER;
	
	Status = BCryptGetProperty(hPrf, L"HashDigestLength", (PUCHAR)&DigestLength, sizeof(DWORD), &ResultLength, 0);
	if(Status < STATUS_SUCCESS)
		return Status;
	
	
	if ((aSalt = (PUCHAR)RtlAllocateHeap(NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap, 0, cbSalt + 4)) == NULL)
		return STATUS_NO_MEMORY;
	
	if ((obuf = (PUCHAR)RtlAllocateHeap(NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap, 0, DigestLength)) == NULL)
	{
		BCryptFree(aSalt); // undocumented function exported from BCrypt - basically a wrapper for RtlFreeHeap with flags = 0 and PEB heap
		return STATUS_NO_MEMORY;
	}
	
	if ((d1 = (PUCHAR)RtlAllocateHeap(NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap, 0, DigestLength)) == NULL)
	{
		BCryptFree(aSalt);
		BCryptFree(obuf);
		return STATUS_NO_MEMORY;
	}

	if ((d2 = (PUCHAR)RtlAllocateHeap(NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap, 0, DigestLength)) == NULL)
	{
		BCryptFree(aSalt);
		BCryptFree(obuf);
		BCryptFree(d1);
		return STATUS_NO_MEMORY;
	}	

	memcpy(aSalt, pbSalt, cbSalt);

	for (count = 1; cbDerivedKey > 0; count++) {
		aSalt[cbSalt + 0] = (count >> 24) & 0xff;
		aSalt[cbSalt + 1] = (count >> 16) & 0xff;
		aSalt[cbSalt + 2] = (count >> 8) & 0xff;
		aSalt[cbSalt + 3] = count & 0xff;
		Status = BCryptHash(hPrf, (PUCHAR)pbPassword, cbPassword, aSalt, cbSalt + 4, d1, DigestLength);
		if (Status < STATUS_SUCCESS)
			goto FreeMemory;
		memcpy(obuf, d1, DigestLength);

		for (i = 1; i < cIterations; i++) {
			Status = BCryptHash(hPrf, (PUCHAR)pbPassword, cbPassword, d1, DigestLength, d2, DigestLength);
		if (Status < STATUS_SUCCESS)
			goto FreeMemory;			
			memcpy(d1, d2, DigestLength);
			for (j = 0; j < DigestLength; j++)
				obuf[j] ^= d1[j];
		}

		r = min(cbDerivedKey, DigestLength);
		memcpy(pbDerivedKey, obuf, r);
		pbDerivedKey += r;
		cbDerivedKey -= r;
	};
	
FreeMemory:	
	memset(aSalt, 0, cbSalt + 4);
	BCryptFree(aSalt);
	memset(d1, 0, DigestLength);
	BCryptFree(d1);
	memset(d2, 0, DigestLength);
    BCryptFree(d2);
	memset(obuf, 0, DigestLength);
	BCryptFree(obuf);
	
	return Status;
}
