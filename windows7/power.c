#include <winuser.h>

HANDLE g_hkPower = 0;

BOOL WINAPI Implementation_PowerClearRequest(
	HANDLE             PowerRequest,
	POWER_REQUEST_TYPE RequestType
)
{
	LONG dwQuantity;
	DWORD dwSize;
	switch (RequestType)
	{
	case PowerRequestDisplayRequired:
		SetThreadExecutionState(SetThreadExecutionState(ES_CONTINUOUS) & ~ES_DISPLAY_REQUIRED);
		WaitForSingleObject(MutexRegistry, INFINITE);
		RegQueryValueExA(g_hkPower, "NumberActiveWakeLocks", NULL, NULL, &dwQuantity, &dwSize);
		--dwQuantity;
		if (dwQuantity < 0)
			dwQuantity = 0;
		RegSetValueExA(g_hkPower, "NumberActiveWakeLocks", NULL, REG_DWORD, &dwQuantity, 4);
		if (!dwQuantity)
			SystemParametersInfoW(SPI_SETSCREENSAVEACTIVE, 1, NULL, SPIF_SENDCHANGE);
		ReleaseMutex(MutexRegistry);
		return TRUE;
	case PowerRequestSystemRequired:
		SetThreadExecutionState(SetThreadExecutionState(ES_CONTINUOUS) & ~ES_SYSTEM_REQUIRED);
		return TRUE;
	case PowerRequestAwayModeRequired:
		SetThreadExecutionState(SetThreadExecutionState(ES_CONTINUOUS) & ~ES_AWAYMODE_REQUIRED);
		return TRUE;
	case PowerRequestExecutionRequired:
		SetThreadExecutionState(SetThreadExecutionState(ES_CONTINUOUS) & ~ES_SYSTEM_REQUIRED);
		return TRUE;
	}
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
}
HANDLE WINAPI Implementation_PowerCreateRequest(
	PREASON_CONTEXT Context
)
{
	DWORD dwQuantity, dwSize;
	WaitForSingleObject(MutexRegistry, INFINITE);

	if (!g_hkPower)
		RegCreateKeyA(HKEY_LOCAL_MACHINE, "Software\\Progwrp", &g_hkPower);
	if (RegQueryValueExA(g_hkPower, "NumberActiveWakeLocks", NULL, NULL, &dwQuantity, &dwSize) != ERROR_SUCCESS)
	{
		dwQuantity = 0;
		RegSetValueExA(g_hkPower, "NumberActiveWakeLocks", 0, REG_DWORD, &dwQuantity, 4);
	}

	ReleaseMutex(MutexRegistry);
	return CreateSemaphoreA(NULL, 0, 1, NULL);
}
BOOL WINAPI Implementation_PowerSetRequest(
	HANDLE             PowerRequest,
	POWER_REQUEST_TYPE RequestType
)
{
	LONG dwQuantity;
	DWORD dwSize;
	ULONG PreviousRequest;
	switch (RequestType)
	{
	case PowerRequestDisplayRequired:
		PreviousRequest = SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
		// MS documentation says that this function has no effect on screensavers, while
		// some GLFW code and issues claim otherwise, but that code has mostly Windows 10 in mind.
		// On XP, this was insufficient. So I am also calling SystemParametersInfo to
		// disable the screensaver.
		SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | PreviousRequest);
		WaitForSingleObject(MutexRegistry, INFINITE);
		RegQueryValueExA(g_hkPower, "NumberActiveWakeLocks", NULL, NULL, &dwQuantity, &dwSize);
		++dwQuantity;
		RegSetValueExA(g_hkPower, "NumberActiveWakeLocks", NULL, REG_DWORD, &dwQuantity, 4);
		if (dwQuantity > 0)
			SystemParametersInfoW(SPI_SETSCREENSAVEACTIVE, 0, NULL, SPIF_SENDCHANGE);
		ReleaseMutex(MutexRegistry);
		return TRUE;
	case PowerRequestSystemRequired:
		PreviousRequest = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
		SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | PreviousRequest);
		return TRUE;
	case PowerRequestAwayModeRequired:
		PreviousRequest = SetThreadExecutionState(ES_CONTINUOUS | ES_AWAYMODE_REQUIRED);
		SetThreadExecutionState(ES_CONTINUOUS | ES_AWAYMODE_REQUIRED | PreviousRequest);
		return TRUE;
	case PowerRequestExecutionRequired:
		PreviousRequest = SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
		SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | PreviousRequest);
		return TRUE;
	}
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
}
