BOOL WINAPI GetPointerType(UINT32 pointerId, POINTER_INPUT_TYPE *pointerType)
/*
   It seems that pointerIds are registered in a kernel mode struct that
   Vista likely does not have.
*/
{
	if(!pointerType || !pointerId || pointerId > 0xFF)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	
	if(pointerId == 1)
		*pointerType = PT_MOUSE;
	
	return TRUE;
}

BOOL WINAPI GetPointerFrameTouchInfo(UINT32 pointerId, UINT32 *pointerCount, POINTER_TOUCH_INFO *touchInfo)
{
	SetLastError(ERROR_DATATYPE_MISMATCH);
	return FALSE;
}

BOOL WINAPI GetPointerFrameTouchInfoHistory(UINT32 pointerId, UINT32 *entriesCount, UINT32 *pointerCount, POINTER_TOUCH_INFO *touchInfo)
{
	SetLastError(ERROR_DATATYPE_MISMATCH);
	return FALSE;
}

BOOL WINAPI GetPointerPenInfo(UINT32 pointerId, POINTER_PEN_INFO *penInfo)
{
	SetLastError(ERROR_DATATYPE_MISMATCH);
	return FALSE;
}

BOOL WINAPI GetPointerPenInfoHistory(UINT32 pointerId, UINT32 *entriesCount, POINTER_PEN_INFO *penInfo)
{
	SetLastError(ERROR_DATATYPE_MISMATCH);
	return FALSE;
}

BOOL SkipPointerFrameMessages(UINT32 pointerId)
{
	return TRUE;
}

BOOL GetPointerDeviceRects(HANDLE device, RECT *pointerDeviceRect, RECT *displayRect)
{
	if(!device || !pointerDeviceRect || !displayRect)
    {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	
	pointerDeviceRect->top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	pointerDeviceRect->left	= GetSystemMetrics(SM_XVIRTUALSCREEN);
	pointerDeviceRect->right = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	pointerDeviceRect->bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	
	displayRect = pointerDeviceRect;
	
	return TRUE;
}