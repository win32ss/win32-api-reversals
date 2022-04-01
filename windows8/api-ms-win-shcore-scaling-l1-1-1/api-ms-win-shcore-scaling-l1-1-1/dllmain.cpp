// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <shtypes.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

HRESULT WINAPI GetScaleFactorForMonitor
( IN HMONITOR       hMon,
  OUT DEVICE_SCALE_FACTOR *pScale)
{
	*pScale = SCALE_100_PERCENT;
	return S_OK;
}



