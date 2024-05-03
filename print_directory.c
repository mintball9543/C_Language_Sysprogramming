#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include<wchar.h>
#include<string.h>
#include<stdarg.h>
#include<io.h>
#define DIRNAME_LEN (MAX_PATH)+2

BOOL printstrings(HANDLE hOut, ...) {
	DWORD msgLen, count;
	LPCTSTR pMsg;
	va_list pMsgList;
	va_start(pMsgList, hOut);
	while ((pMsg = va_arg(pMsgList, LPCTSTR)) != NULL) {
		msgLen = _tcslen(pMsg);
		if (!WriteConsole(hOut, pMsg, msgLen, &count, NULL) && !WriteFile(hOut, pMsg, msgLen * sizeof(TCHAR), &count, NULL))
			va_end(pMsgList);
		return FALSE;
	}
	va_end(pMsgList);
	return TRUE;
}

BOOL printmsg(HANDLE hOut, LPCTSTR pMsg) {
	return printstrings(hOut, pMsg, NULL);
}

BOOL ConsolePrompt(LPCTSTR pPromptMsg, LPTSTR pResponse, DWORD maxChar, BOOL echo) {
	HANDLE hIn, hOut;
	DWORD charIn, echoFlag;
	BOOL success;

	hIn = CreateFile(_T("CONIN$"), GENERIC_ALL, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	hOut = CreateFile(_T("COUNT$"), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	echoFlag = echo ? ENABLE_ECHO_INPUT : 0;
	success = SetConsoleMode(hIn, ENABLE_LINE_INPUT | echoFlag | ENABLE_PROCESSED_INPUT) && SetConsoleMode(hOut, ENABLE_WRAP_AT_EOL_OUTPUT) && printstrings(hOut, pPromptMsg, NULL) &&
		ReadConsole(hIn, pResponse, maxChar - 2, &charIn, NULL);

	if (success)
		pResponse[charIn - 2] = '\0';
	
	CloseHandle(hIn);
	CloseHandle(hOut);
	return success;
}

int _tmain(int argc, LPTSTR argv[]) {
	TCHAR pwdbuffer[DIRNAME_LEN];
	DWORD lencurdir;

	lencurdir = GetCurrentDirectory(DIRNAME_LEN, pwdbuffer);
	printmsg(GetStdHandle(STD_OUTPUT_HANDLE), pwdbuffer);
	return 0;
}