#include<stdio.h>
#include<Windows.h>
#include<stdbool.h>
#include<tchar.h>
#include<wchar.h>
#include<strsafe.h>

BOOL GetLastWriteTime(HANDLE hFile, LPTSTR lpszString, DWORD dwSize) {
	FILETIME ftCreate, ftAccess, ftWrite;
	SYSTEMTIME stUTC, stLocal;
	DWORD dwRet;

	if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
	return FALSE;

	FileTimeToSystemTime(&ftWrite, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
	
	dwRet = StringCchPrintf(lpszString, dwSize, TEXT("%02d/%02d/%d  %02d:%02d"), stLocal.wMonth, stLocal.wDay, stLocal.wYear, stLocal.wHour, stLocal.wMinute);

	if (S_OK == dwRet)
		return TRUE;
	else return FALSE;
}

int _tmain(int argc, TCHAR* argv[]) {
	HANDLE hFile;
	TCHAR szBuf[MAX_PATH];

	if (argc != 2) {
		printf("fdsafsda");
		return 0;
	}

	hFile = CreateFile(argv[1], GENERIC_ALL, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		printf("%d\n", GetLastError());
	if (GetLastWriteTime(hFile, szBuf, MAX_PATH))
		_tprintf(TEXT("Last write time is: %s\n"), szBuf);

	CloseHandle(hFile);
}