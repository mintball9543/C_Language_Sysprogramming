#include<stdio.h>
#include<Windows.h>
#include<time.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT imsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI Thread(LPVOID lpParam);
DWORD WINAPI Thread2(LPVOID lpParam);

HINSTANCE hinst;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hprevInstance, LPSTR cmdLine, int nCmdShow) {
	
	MSG msg;
	WNDCLASS wc;
	hinst = hInstance;
	HWND hwnd;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = "thread";
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	RegisterClass(&wc);
	hwnd = CreateWindow("thread", "thread", WS_OVERLAPPEDWINDOW|WS_VSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, 0, hInstance, 0);
	
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT imsg, WPARAM wParam, LPARAM lParam) {
	int i = 100;
	int j = 100;
	HDC hdc;
	PAINTSTRUCT ps;
	HANDLE hthread[2];

	switch (imsg) {
	case WM_CREATE:
		CreateWindow("button", "Thread1", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD, 10, 10, 100, 50, hwnd, 1000, hinst, 0);
		CreateWindow("button", "Thread2", BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD, 120, 10, 100, 50, hwnd, 1001, hinst, 0);
		break;
	case WM_COMMAND:
		switch (wParam) {
		case 1000:
			/*hdc = GetDC(hwnd);
			hthread[0]=CreateThread(NULL, 0, Thread, (LPVOID)hdc, 0, NULL);
			WaitForSingleObject(hthread[0], INFINITE);*/
			SetTimer(hwnd, 1, 500, NULL);
			break;
		case 1001:
			//hthread[1]=CreateThread(NULL, 0, Thread2, hdc, 0, NULL);
			//WaitForMultipleObjects(2, hthread, TRUE, INFINITE);
			SetTimer(hwnd, 2, 500, NULL);
			break;
		}
		break;
	case WM_TIMER:
		switch (wParam) {
		case 1:
			hdc=GetDC(hwnd);
			hthread[0]=CreateThread(NULL, 0, Thread, hdc, 0, NULL);
			WaitForSingleObject(hthread[0], INFINITE);
			TerminateThread(hthread[0], 0);
			break;
		case 2:
			hdc=GetDC(hwnd);
			hthread[1]=CreateThread(NULL, 0, Thread2, hdc, 0, NULL);
			WaitForSingleObject(hthread[1], INFINITE);
			TerminateThread(hthread[1], 0);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, imsg, wParam, lParam);
}

DWORD WINAPI Thread(LPVOID lpParam) {
	static int i = 100;
	HDC hdc = (HDC)lpParam;
	//TextOut(hdc, 100, 100, "test", strlen("test"));
	char temp[20] = { 0 };
	sprintf(temp, "Thread1: %d", i);
	TextOut(hdc, 100, 100, temp, strlen(temp));
	i++;
	/*if (clock() - time >= 500) {
		char* temp = malloc(sizeof(char) * 20);
		sprintf(temp, "Thread1: %d", i);
		TextOut(hdc, 10, i * 20, &temp, strlen(temp));
		i++;
		time = clock();
	}*/

	//ReleaseDC(hwnd, hdc);

	return 0;
}


DWORD WINAPI Thread2(LPVOID lpParam) {
	static int j = 100;
	HDC hdc = (HDC)lpParam;
	char temp[20] = { 0 };
	sprintf(temp, "Thread2: %d", j);
	TextOut(hdc, 200, 100, temp, strlen(temp));
	j++;

	return 0;
}