#include<Windows.h>
#include<stdio.h>

#define THREADCOUNT 4
#define DLL_NAME TEXT("testdll")

VOID ErrorExit(LPSTR);

extern BOOL WINAPI StoreData(DWORD dw);
extern BOOL WINAPI GetData(DWORD* pdw);

DWORD WINAPI ThreadFunc(VOID) {
	int i;

	if (!StoreData(GetCurrentThreadId()))
		ErrorExit("StoreData Error");

	for (i = 0; i < THREADCOUNT; i++) {
		DWORD dwOut;
		if (!GetData(&dwOut))
			ErrorExit("GetData error");
		if (dwOut != GetCurrentThreadId())
			printf("thread %d: data is incorrect (%d)\n", GetCurrentThreadId());
		else printf("thread %d: data is correct \n", GetCurrentThreadId());
		Sleep(0);
	}

	return 0;
}

int main(VOID) {
	DWORD IDThread;
	HANDLE hThread[THREADCOUNT];
	int i;
	HMODULE hm;

	hm = LoadLibrary("Project4222.dll");
	if (!hm)
		ErrorExit("DLL failed to load");

	for (i = 0; i < THREADCOUNT; i++) {
		hThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc, NULL, 0, &IDThread);

		if (hThread[i] == NULL)
			ErrorExit("CreateThread error \n");
	}

	WaitForMultipleObjects(THREADCOUNT, hThread, TRUE, INFINITE);

	FreeLibrary(hm);

	return 0;
}

VOID ErrorExit(LPSTR lpszMessage) {
	fprintf(stderr, "%s\n", lpszMessage);
	ExitProcess(0);
}