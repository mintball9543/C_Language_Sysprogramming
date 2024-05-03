#include<Windows.h>
#include<stdio.h>
#include<tchar.h>

static BOOL WINAPI Handler(DWORD cntrlEvent);
static BOOL exitFlag = FALSE;
int _tmain(int argc, LPTSTR argv[]) {
	SetConsoleCtrlHandler(Handler, TRUE);

	while (!exitFlag) {
		Sleep(5000);
		Beep(1000,250);
	}
	_tprintf(_T("Stopping the program as requested.\n"));

	return 0;
}

BOOL WINAPI Handler(DWORD cntrlEvent) {
	exitFlag = TRUE;
	switch (cntrlEvent) {
	case CTRL_C_EVENT:
		_tprintf(_T("Ctrl-C. Leaving in <= 5 seconds.\n"));
		exitFlag = TRUE;
		Sleep(4000);
		_tprintf(_T("Leaving handler in 1 second or less.\n"));
		return TRUE;
	case CTRL_CLOSE_EVENT:
		_tprintf(_T("Close event. Leaving in <= 5 seconds.\n"));
		exitFlag = TRUE;
		Sleep(4000);
		_tprintf(_T("Leaving handler in <= 1 seconds.\n"));
		return TRUE;
	case CTRL_SHUTDOWN_EVENT:
		_tprintf(_T("ShutDown event. Leaving in <= 5 seconds.\n"));
		exitFlag = TRUE;
		Sleep(4000);
		_tprintf(_T("Leaving handler in <= 1 seconds.\n"));
		return TRUE;
	case CTRL_LOGOFF_EVENT:
		_tprintf(_T("Close event. Leaving in <= 5 seconds.\n"));
		exitFlag = TRUE;
		Sleep(4000);
		_tprintf(_T("Leaving handler in <= 1 seconds.\n"));
		return TRUE;
	default:
		_tprintf(_T("Event: %d. Leaving in <= 5 seconds.\n"), cntrlEvent);
		exitFlag = TRUE;
		Sleep(4000);
		_tprintf(_T("Leaving handler in <= 1 second.\n"));
		return TRUE;
	}
}