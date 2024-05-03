#include<stdio.h>
#include<Windows.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT imsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI thread(LPVOID lpParam);
DWORD WINAPI thread2(LPVOID lpParam);

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
	hwnd = CreateWindow("thread", "thread", WS_OVERLAPPEDWINDOW | WS_VSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, 0, hInstance, 0);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT imsg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	RECT rt;
	static char str[1024] = { 0, };
	static int idx = 0;
	static HANDLE threadH;
	PAINTSTRUCT ps;
	switch (imsg) {
	case WM_CREATE:
		//threadH = CreateThread(NULL, 0, thread, GetDC(hwnd), 0, NULL); //ani
		//WaitForSingleObject(threadH, INFINITE);
		SetTimer(hwnd, 1, 50, NULL);
		SetTimer(hwnd, 2, 50, NULL);
		break;
	case WM_LBUTTONDOWN:
		//threadH = CreateThread(NULL, 0, thread, GetDC(hwnd), 0, NULL); //ani
		//WaitForSingleObject(threadH, INFINITE);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		TextOut(hdc, 10, 200, str, strlen(str));
		EndPaint(hwnd, &ps);
		break;
	case WM_CHAR:
		str[idx] = wParam;
		idx++;
		InvalidateRgn(hwnd, NULL, TRUE);
		break;
	case WM_TIMER:
		switch (wParam) {
		case 1:
			threadH = CreateThread(NULL, 0, thread, hwnd, 0, NULL); //ani
			WaitForSingleObject(threadH, INFINITE);
			TerminateThread(threadH, 0);
			break;
		case 2:
			InvalidateRgn(hwnd, NULL, TRUE);
		}
		//SuspendThread(threadH);
		//ResumeThread(threadH);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, imsg, wParam, lParam);
}

DWORD WINAPI thread(LPVOID lpParam) {
	HWND hwnd = (HWND)lpParam;
	HDC hdc = GetDC(hwnd);
	RECT rt;
	static int reverse = 0;
	GetClientRect(hwnd, &rt);

	static int left=10, top=10, right=60, bottom=60;
	
	Rectangle(hdc, left, top, right, bottom);

	if (right >= rt.right - 10)
		reverse = 1;
	else if (left <= rt.left + 10)
		reverse = 0;

	if (reverse) {
		left -= 10;
		right -= 10;
	}
	else {
		left += 10;
		right += 10;
	}

	return 0;
}

DWORD WINAPI thread2(LPVOID lpParam) {

}