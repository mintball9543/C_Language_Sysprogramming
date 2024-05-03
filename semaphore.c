#include<stdio.h>
#include<Windows.h>

#define MAX_SEM_COUNT 10
#define THREADCOUNT 12

HANDLE ghSemaphore;

DWORD WINAPI ThreadProc(LPVOID);

int main(void) {
	HANDLE aThread[THREADCOUNT];
	DWORD ThreadID;
	int i;

	ghSemaphore = CreateSemaphore(NULL, MAX_SEM_COUNT, MAX_SEM_COUNT, NULL);

	if (ghSemaphore == NULL) {
		printf("CreateSemaphore error: %d\n", GetLastError());
		return 1;
	}

	for (i = 0; i < THREADCOUNT; i++) {
		aThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProc, NULL, 0, &ThreadID);
		if (aThread[i] == NULL) {
			printf("CreateThread error: %d\n", GetLastError());
			return 1;
		}
	}

	WaitForMultipleObjects(THREADCOUNT, aThread, TRUE, INFINITE);

	for (i = 0; i < THREADCOUNT; i++)
		CloseHandle(aThread[i]);

	CloseHandle(ghSemaphore);

	return 0;
}

DWORD WINAPI ThreadProc(LPVOID lpParam) {
	UNREFERENCED_PARAMETER(lpParam);

	DWORD dwWaitResult;
	BOOL bContinue = TRUE;

	while (bContinue) {
		dwWaitResult = WaitForSingleObject(ghSemaphore, 0L);
		switch (dwWaitResult) {
		case WAIT_OBJECT_0:
			printf("Thread %d: wait succeeded\n", GetCurrentThreadId());
			bContinue = FALSE;

			Sleep(5);

			if (!ReleaseSemaphore(ghSemaphore, 1, NULL)) {
				printf("ReleaseSemaphore error: %d\n", GetLastError());
			}
			break;

		case WAIT_TIMEOUT:
			printf("Thread %d: wait timed out\n", GetCurrentThreadId());
			break;
		}
	}
	return TRUE;
}