BOOL WINAPI AdjustWindowRectExForDpi(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi)
{
   if(!AdjustWindowRectEx(lpRect, dwStyle, bMenu, dwExStyle))
       return FALSE;
   else
   {
	   lpRect->left *= dpi/96;
	   lpRect->top *= dpi/96;
	   lpRect->right *= dpi/96;
	   lpRect->bottom *= dpi/96;
	   
	   return TRUE;
   }
}

UINT WINAPI GetDpiForWindow(HWND hwnd)
{
	HDC hDC;
	int result;
	if(IsProcessDPIAware())
	{
		hDC = GetDC(hwnd);
		if(!hwnd)
			return 0;
		result = GetDeviceCaps(hDC, LOGPIXELSX);
		ReleaseDC(hwnd, hDC);
		return result;
	}
	else
		return 96;
}

int WINAPI GetSystemMetricsForDpi(int nIndex, UINT dpi)
{
	int result;
	
	result = GetSystemMetrics(nIndex);
	
	switch(nIndex)
	{
		case SM_CXBORDER:
		case SM_CXMAXTRACK:
		case SM_CXMIN:
		case SM_CXMINTRACK:
		case SM_CYMAXTRACK:
		case SM_CYMIN:
		case SM_CYMINTRACK:
		case SM_CXICON:
		case SM_CXICONSPACING:
		case SM_CXSMICON:
		case SM_CYICON:
		case SM_CYICONSPACING:
		case SM_CYSMICON:
           return result * (dpi/96);
		
		default:
		   return result;
	}
}

DPI_AWARENESS WINAPI GetAwarenessFromDpiAwarenessContext(DPI_AWARENESS_CONTEXT value)
{
	if(value > DPI_AWARENESS_CONTEXT_UNAWARE || value < DPI_AWARENESS_CONTEXT_SYSTEM_AWARE)
		return DPI_AWARENESS_INVALID;
	else if (value == DPI_AWARENESS_CONTEXT_UNAWARE)
		return DPI_AWARENESS_UNAWARE;
	else
		return DPI_AWARENESS_SYSTEM_AWARE;
}

DPI_AWARENESS_CONTEXT WINAPI GetWindowDpiAwarenessContext(HWND hwnd)
{
	if(IsProcessDPIAware())
		return DPI_AWARENESS_CONTEXT_SYSTEM_AWARE;
	else
		return DPI_AWARENESS_CONTEXT_UNAWARE;
}


BOOL WINAPI EnableNonClientDpiScaling(HWND hwnd)
{
	SetLastError(ERROR_NOT_SUPPORTED);
	return FALSE;
}

BOOL WINAPI SystemParametersInfoForDpi(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni, UINT dpi)
{
	LOGFONTW* logfont;
	ICONMETRICSW* iconmetrics;
	NONCLIENTMETRICSW* nonclientmetrics;
	
	if(!SystemParametersInfo(uiAction, uiParam, pvParam, fWinIni))
		return FALSE;
	else
	{
		if(uiAction == SPI_GETICONTITLELOGFONT)
		{
			logfont = (LOGFONTW*) pvParam;
			
			logfont->lfHeight *= dpi/96;
			logfont->lfWidth *= dpi/96;
			return TRUE;
		}
		
		if(uiAction == SPI_GETICONMETRICS)
		{
			iconmetrics = (ICONMETRICSW*) pvParam;
			
			iconmetrics->iHorzSpacing *= dpi/96;
			iconmetrics->iVertSpacing *= dpi/96;
			return TRUE;
		}
		
		if(uiAction == SPI_GETNONCLIENTMETRICS)
		{
			nonclientmetrics = (NONCLIENTMETRICSW*) pvParam;
			
			nonclientmetrics->iBorderWidth *= dpi/96;
			nonclientmetrics->iScrollWidth *= dpi/96;
			nonclientmetrics->iScrollHeight *= dpi/96;
			nonclientmetrics->iCaptionWidth *= dpi/96;
			nonclientmetrics->iCaptionHeight *= dpi/96;
			
			nonclientmetrics->iSmCaptionWidth *= dpi/96;
			nonclientmetrics->iSmCaptionHeight *= dpi/96;
			nonclientmetrics->iMenuWidth *= dpi/96;
			nonclientmetrics->iMenuHeight *= dpi/96;
			nonclientmetrics->iPaddedBorderWidth *= dpi/96;
			return TRUE;
		}
	}
}

