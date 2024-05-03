#include<stdio.h>
#include<Windows.h>

#define THREADCOUNT 4
char temp[126] = { 0 };
HANDLE ghWriteEvent;
HANDLE ghTreads[THREADCOUNT];

DWORD WINAPI ThreadProc(LPVOID);

void CreateEventsAndThreads(void) {
	int i;
	DWORD dwThreadID;

	ghWriteEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("WriteEvent"));

	if (ghWriteEvent == NULL) {
		printf("CreateEvent failed (%d)\n", GetLastError());
		return;
	}

	for (i = 0; i < THREADCOUNT; i++) {
		ghTreads[i] = CreateThread(NULL, 0, ThreadProc, NULL, 0, &dwThreadID);

		if (ghTreads[i] == NULL) {
			printf("CreateThread failed (%d)\n", GetLastError());
			return;
		}
	}
}

void WriteToBuffer(VOID) {
	printf("Main thread writing to the shared buffer...\n");
	strcpy(temp, "test");

	if (!SetEvent(ghWriteEvent)) {
		printf("SetEvent failed (%d)\n", GetLastError());
		return;
	}
}

void CloseEvents() {
	CloseHandle(ghWriteEvent);
}

int main(void) {
	DWORD dwWaitResult;
	CreateEventsAndThreads();
	WriteToBuffer();
	printf("Main thread waiting for threads to exit...\n");

	dwWaitResult = WaitForMultipleObjects(THREADCOUNT, ghTreads, TRUE, INFINITE);

	switch (dwWaitResult) {
	case WAIT_OBJECT_0:
		printf("All threads ended, cleaning up for application exit...\n");
		break;

	default:
		printf("WaitForMultipleObjects failed (%d)\n", GetLastError());
		return 1;
	}
	CloseEvents();

	return 0;
}

DWORD WINAPI ThreadProc(LPVOID lpParam) {
	UNREFERENCED_PARAMETER(lpParam);
	DWORD dwWaitResult;

	printf("Thread %d waiting for write event...\n", GetCurrentThreadId());

	dwWaitResult = WaitForSingleObject(ghWriteEvent, INFINITE);

	switch (dwWaitResult) {
	case WAIT_OBJECT_0:
		printf("Thread %d reading from buffer\n", GetCurrentThreadId());
		break;

	default:
		printf("Wait error (%d)\n", GetLastError());
		return 0;
	}
	printf("Thread %d exiting \n", GetCurrentThreadId());
	return 1;
}