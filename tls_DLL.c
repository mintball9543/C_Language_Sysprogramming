#include<Windows.h>


static DWORD dwTlsIndex;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	LPVOID lpvData;
	BOOL fIgnore;

	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		if ((dwTlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES)
			return FALSE;
	case DLL_THREAD_ATTACH:
		lpvData = (LPVOID)LocalAlloc(LPTR, 256);
		if (lpvData != NULL)
			fIgnore = TlsSetValue(dwTlsIndex, lpvData);

		break;
	case DLL_THREAD_DETACH:
		lpvData = TlsGetValue(dwTlsIndex);
		if (lpvData != NULL)
			LocalFree((HLOCAL)lpvData);
		break;
	case DLL_PROCESS_DETACH:
		lpvData = TlsGetValue(dwTlsIndex);
		if (lpvData != NULL)
			LocalFree((HLOCAL)lpvData);

		TlsFree(dwTlsIndex);
		break;
	default:
		break;
	}
	return TRUE;
	UNREFERENCED_PARAMETER(hinstDLL);
	UNREFERENCED_PARAMETER(lpvReserved);
}

__declspec(dllexport) BOOL WINAPI StoreData(DWORD dw) {
	LPVOID lpvData;
	DWORD* pData;

	lpvData = TlsGetValue(dwTlsIndex);
	if (lpvData == NULL) {
		lpvData = (LPVOID)LocalAlloc(LPTR, 256);
		if (lpvData == NULL)
			return FALSE;
		if (!TlsSetValue(dwTlsIndex, lpvData))
			return FALSE;
	}
	pData = (DWORD*)lpvData;

	(*pData) = dw;
	return TRUE;
}

__declspec(dllexport) BOOL WINAPI GetData(DWORD* pdw) {
	LPVOID lpvData;
	DWORD* pData;

	lpvData = TlsGetValue(dwTlsIndex);
	if (lpvData == NULL)
		return FALSE;

	pData = (DWORD*)lpvData;
	(*pdw) = (*pData);
	return TRUE;
}