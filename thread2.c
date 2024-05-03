#include<stdio.h>
#include<Windows.h>
#include<time.h>

DWORD WINAPI Thread(LPVOID lpParam);
DWORD WINAPI Thread2(LPVOID lpParam);

int main() {
	int i=0;
	HANDLE Thread_HANDLE[2];

	Thread_HANDLE[0] = CreateThread(NULL, 0, Thread, i, 0, NULL);
	Thread_HANDLE[1] = CreateThread(NULL, 0, Thread2, ++i, 0, NULL);

	WaitForMultipleObjects(2, Thread_HANDLE, TRUE, INFINITE);


	return 0;
}

DWORD WINAPI Thread(LPVOID lpParam) {
	int i=0;
	unsigned t = clock();

	while (1) {
		if (clock() - t >= 500) {
			printf("Thread1: %d\t\t\t", i++);
			t = clock();
		}
	}
	return 0;
}

DWORD WINAPI Thread2(LPVOID lpParam) {
	int i=0;
	unsigned t = clock();

	while (1) {
		if (clock() - t >= 500) {
			printf("Thread2: %d\n", i++);
			t = clock();
		}
	}

	return 0;
}