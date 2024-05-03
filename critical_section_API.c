#include<stdio.h>
#include<Windows.h>

struct Person {
	char name[20];
	char id[20];
	char phone[20];
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT imsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI Thread(LPVOID lpParam);
DWORD WINAPI Thread2(LPVOID lpParam);
DWORD WINAPI control(LPVOID lpParam);

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
	wc.lpszClassName = "critical_section";
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	RegisterClass(&wc);
	hwnd = CreateWindow("critical_section", "critical_section", WS_OVERLAPPEDWINDOW | WS_VSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, 0, hInstance, 0);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
	HANDLE edit[3];
	CRITICAL_SECTION cs;
	struct Person p1;
	char print[1024] = { 0 };
	int flag = 0;
	HANDLE fp;
LRESULT CALLBACK WndProc(HWND hwnd, UINT imsg, WPARAM wParam, LPARAM lParam) {
	static HANDLE t,t2;
	char temp[100];
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rt = { 200,50,600,500 };
	switch (imsg) {
	case WM_CREATE:
		InitializeCriticalSection(&cs);
		CreateWindow("static", "이름: ", SS_CENTER | WS_CHILD | WS_VISIBLE, 200, 400, 50, 20, hwnd, 1000, hinst, 0);
		CreateWindow("static", "학번: ", SS_CENTER | WS_CHILD | WS_VISIBLE, 200, 430, 50, 20, hwnd, 1001, hinst, 0);
		CreateWindow("static", "전번: ", SS_CENTER | WS_CHILD | WS_VISIBLE, 200, 460, 50, 20, hwnd, 1002, hinst, 0);
		edit[0]=CreateWindow("edit", "", ES_LEFT | WS_CHILD | WS_VISIBLE | WS_BORDER, 260, 400, 100, 20, hwnd, 2000, hinst, 0);
		edit[1]=CreateWindow("edit", "", ES_LEFT | WS_CHILD | WS_VISIBLE | WS_BORDER, 260, 430, 100, 20, hwnd, 2001, hinst, 0);
		edit[2]=CreateWindow("edit", "", ES_LEFT | WS_CHILD | WS_VISIBLE | WS_BORDER, 260, 460, 100, 20, hwnd, 2002, hinst, 0);
		CreateWindow("button", "입력", BS_CENTER | WS_CHILD | WS_VISIBLE, 380, 430, 50, 30, hwnd, 3000, hinst, 0);
		CreateWindow("button", "닫기", BS_CENTER | WS_CHILD | WS_VISIBLE, 430, 430, 50, 30, hwnd, 3001, hinst, 0);
		SetTimer(hwnd, 1, 500, NULL);
		CreateThread(NULL, 0, Thread2, NULL, 0, NULL);
		break;
	case WM_COMMAND:
		switch (wParam) {
		case 3000:
			//MessageBox(NULL, GetWindowText(edit[0], ));
			GetWindowText(edit[0], p1.name, 20);
			GetWindowText(edit[1], p1.id, 20);
			GetWindowText(edit[2], p1.phone, 20);
			CreateThread(NULL, 0, control, edit, 0, NULL);


			break;
		case 3001:
			flag++;
			break;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		//TextOut(hdc, 200, 100, print, strlen(print));
		DrawText(hdc, print, strlen(print), &rt, DT_LEFT | DT_TOP);
		EndPaint(hwnd, &ps);
		break;
	case WM_TIMER:
		
		InvalidateRgn(hwnd, NULL, TRUE);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, imsg, wParam, lParam);
}

DWORD WINAPI control(LPVOID lpParam) {
	HANDLE t;

	t=CreateThread(NULL, 0, Thread, NULL, 0, NULL);
	WaitForSingleObject(t, INFINITE);
	TerminateThread(t, 0);
	return 0;
}

DWORD WINAPI Thread(LPVOID lpParam) {
	EnterCriticalSection(&cs);
	DWORD r;

	char str[1024] = { 0 };
	fp = CreateFile("student.txt", GENERIC_ALL, NULL, NULL, OPEN_ALWAYS, NULL, NULL);
	ReadFile(fp, str, 1024, &r, NULL);
	sprintf(str, "이름: %s, 학번: %s, 전화번호: %s \n", p1.name, p1.id, p1.phone);
	
	
	WriteFile(fp, str, strlen(str), &r, NULL);
	
	while (1) {
		if (flag == 1) {
			CloseHandle(fp);
			flag = 0;
			break;
		}
	}

	LeaveCriticalSection(&cs);
	return 0;
}


DWORD WINAPI Thread2(LPVOID lpParam) {
	while (1) {
		EnterCriticalSection(&cs);

		//HANDLE fp;
		DWORD r;
		fp = CreateFile("student.txt", GENERIC_ALL, NULL, NULL, OPEN_ALWAYS, NULL, NULL);
		ReadFile(fp, print, 1024, &r, NULL);

		CloseHandle(fp);
		LeaveCriticalSection(&cs);
	}
	return 0;
}
