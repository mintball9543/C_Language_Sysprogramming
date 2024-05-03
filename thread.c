#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include<strsafe.h>

#define MAX_THREADS 2048
#define BUF_SIZE 255

DWORD WINAPI MyThreadFunction(LPVOID lpParam);
DWORD WINAPI MyThreadFunction2(LPVOID lpParam);
void ErrorHandler(LPTSTR lpszFunction);

typedef struct MyData {
	int val1;
	int val2;
}MYDATA, *PMYDATA;

int _tmain() {
	PMYDATA pDataArray[MAX_THREADS];
	PMYDATA pDataArray2[MAX_THREADS];
	DWORD dwThreadIdArray[MAX_THREADS];
	DWORD dwThreadIdArray2[MAX_THREADS];
	HANDLE hTreadArray[MAX_THREADS];
	HANDLE hTreadArray2[MAX_THREADS];

	for (int i = 0;; i++) {
		if (i == 500) i = 0;
		pDataArray[i] = (PMYDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MYDATA));
		pDataArray2[i] = (PMYDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MYDATA));

		if (pDataArray[i] == NULL)
			ExitProcess(2);

		pDataArray[i]->val1 = i;
		pDataArray[i]->val2 = i + 100;

		pDataArray2[i]->val1 = i;
		pDataArray2[i]->val2 = i + 100;

		hTreadArray[i] = CreateThread(NULL, 0, MyThreadFunction, pDataArray[i], 0, &dwThreadIdArray[i]);
		hTreadArray2[i] = CreateThread(NULL, 0, MyThreadFunction2, pDataArray2[i], 0, &dwThreadIdArray2[i]);

		if (hTreadArray[i] == NULL) {
			ErrorHandler(TEXT("CreateThread"));
			ExitProcess(3);
		}
	}

	WaitForMultipleObjects(MAX_THREADS, hTreadArray, TRUE, INFINITE);
	WaitForMultipleObjects(MAX_THREADS, hTreadArray2, TRUE, INFINITE);

	for (int i = 0; i < MAX_THREADS; i++) {
		CloseHandle(hTreadArray[i]);
		CloseHandle(hTreadArray2[i]);
		if (pDataArray[i] != NULL) {
			HeapFree(GetProcessHeap(), 0, pDataArray[i]);
			HeapFree(GetProcessHeap(), 0, pDataArray2[i]);
			pDataArray[i] = NULL;
			pDataArray2[i] = NULL;
		}
	}
	return 0;
}

DWORD WINAPI MyThreadFunction(LPVOID lpParam) {
	HANDLE hStdout;
	PMYDATA pDataArray;

	TCHAR msgBuf[BUF_SIZE];
	size_t cchStringSize;
	DWORD dwChars;

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdout == INVALID_HANDLE_VALUE)
		return 1;

	pDataArray = (PMYDATA)lpParam;

	StringCchPrintf(msgBuf, BUF_SIZE, TEXT("Parameters = %d, %d"), pDataArray->val1, pDataArray->val2);
	StringCchLength(msgBuf, BUF_SIZE, &cchStringSize);
	WriteConsole(hStdout, msgBuf, (DWORD)cchStringSize, &dwChars, NULL);

	return 0;
}

DWORD WINAPI MyThreadFunction2(LPVOID lpParam) {
	HANDLE hStdout;
	PMYDATA pDataArray;

	TCHAR msgBuf[BUF_SIZE];
	size_t cchStringSize;
	DWORD dwChars;

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdout == INVALID_HANDLE_VALUE)
		return 1;

	pDataArray = (PMYDATA)lpParam;

	StringCchPrintf(msgBuf, BUF_SIZE, TEXT("\t\t\t\tParameters = %d, %d\n"), pDataArray->val1, pDataArray->val2);
	StringCchLength(msgBuf, BUF_SIZE, &cchStringSize);
	WriteConsole(hStdout, msgBuf, (DWORD)cchStringSize, &dwChars, NULL);

	return 0;
}

void ErrorHandler(LPTSTR lpszFunction) {
	/*LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("%s failed with error %d: %s"), lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);*/
}