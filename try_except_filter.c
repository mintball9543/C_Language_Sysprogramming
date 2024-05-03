#include<stdio.h>
#include<Windows.h>
#include<wchar.h>
#include<tchar.h>
#include<stdarg.h>
#include<float.h>

VOID ReportError(LPCTSTR UserMessage, DWORD ExitCode, BOOL PrintErrorMsg);
VOID ReportException(LPCTSTR UserMessage, DWORD ExceptionCode);
BOOL PrintStrings(HANDLE hOut, ...);
BOOL PrintMsg(HANDLE hOut, LPCTSTR pMsg);
DWORD Filter(LPEXCEPTION_POINTERS, LPDWORD);
double x = 1.0, y = 0.0;

int _tmain(int argc, LPTSTR argv[]) {
	DWORD eCategory, i = 0, ix, iy = 0;
	LPDWORD pNull = NULL;
	BOOL done = FALSE;
	DWORD fpOld, fpNew;
	fpOld = _controlfp(0, 0);
	fpNew = fpOld & ~(EM_OVERFLOW | EM_UNDERFLOW | EM_INEXACT | EM_ZERODIVIDE | EM_DENORMAL | EM_INVALID);
	_controlfp(fpNew, MCW_EM);

	while (!done) __try {
		_tprintf(_T("Enter exception type: "));
		_tprintf(_T("1: Mem, 2: Int, 3: Flt 4: User 5: __leave"));
		_tscanf(_T("%d"), &i);

		__try {
			switch (i) {
			case 1:
				ix = *pNull; *pNull = 5; break;
			case 2:
				ix = ix / iy; break;
			case 3:
				x = x / y;
				_tprintf(_T("x = %20e\n"), x); break;
			case 4:
				ReportException(_T("User exception"), 1); break;
			case 5:
				__leave;
			default:
				done = TRUE;
			}
		}

		__except (Filter(GetExceptionInformation(), &eCategory)) {
			switch (eCategory) {
			case 0:
				_tprintf(_T("Unknown Exception\n")); break;
			case 1:
				_tprintf(_T("Memory Ref Exception\n")); continue;
			case 2:
				_tprintf(_T("Integer Exception\n")); break;
			case 3:
				_tprintf(_T("Floating-point Exception\n"));
				_clearfp(); break;
			case 10:
				_tprintf(_T("User Exception\n")); break;
			default:
				_tprintf(_T("Unknown Exception\n")); break;
			}
			_tprintf(_T("End of handler\n"));
		}
	}

	__finally {
		_tprintf(_T("Abnormal Termination?: %d\n"), AbnormalTermination());
	}
	_controlfp(fpOld, 0xFFFFFFFF);
	return 0;
}

static DWORD Filter(LPEXCEPTION_POINTERS pExp, LPDWORD eCategory) {
	DWORD exCode, readWrite, virtAddr;
	exCode = pExp->ExceptionRecord->ExceptionCode;
	_tprintf(_T("Filter. exCode: %x\n"), exCode);
	if ((0x20000000 & exCode) != 0) {
		*eCategory = 10;
		return EXCEPTION_EXECUTE_HANDLER;
	}

	switch (exCode) {
	case EXCEPTION_ACCESS_VIOLATION:
		readWrite = pExp->ExceptionRecord->ExceptionInformation[0];
		virtAddr = pExp->ExceptionRecord->ExceptionInformation[1];
		_tprintf(_T("Access Violation. Read/Write/Exec: %d. Address: %x\n"), readWrite, virtAddr);
		*eCategory = 1;
		return EXCEPTION_EXECUTE_HANDLER;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_INT_OVERFLOW:
		*eCategory = 2;
		return EXCEPTION_EXECUTE_HANDLER;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_OVERFLOW:
		_tprintf(_T("Flt Exception - large result.\n"));
		*eCategory = 3;
		_clearfp();
		return EXCEPTION_EXECUTE_HANDLER;

	default:
		*eCategory = 0;
		return EXCEPTION_CONTINUE_SEARCH;
	}
}


VOID ReportError(LPCTSTR UserMessage, DWORD ExitCode, BOOL PrintErrorMsg)

/* General-purpose function for reporting system errors.
	Obtain the error number and turn it into the system error message.
	Display this information and the user-specified message to the standard error device.
	UserMessage:		Message to be displayed to standard error device.
	ExitCode:		0 - Return.
				\x11> 0 - ExitProcess with this code.
	PrintErrorMessage:	Display the last system error message if this flag is set. */
{
	DWORD eMsgLen, ErrNum = GetLastError();
	LPTSTR lpvSysMsg;
	HANDLE hStdErr;
	hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	PrintMsg(hStdErr, UserMessage);
	if (PrintErrorMsg) {
		eMsgLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM, NULL,
			ErrNum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpvSysMsg, 0, NULL);
		PrintStrings(hStdErr, TEXT("\n"), lpvSysMsg, TEXT("\n"), NULL);
		HeapFree(GetProcessHeap(), 0, lpvSysMsg);
		/* Explained in Chapter 6. */

	}
	CloseHandle(hStdErr);
	if (ExitCode > 0)
		ExitProcess(ExitCode);
	else
		return;
}

VOID ReportException(LPCTSTR UserMessage, DWORD ExceptionCode)

/* Report as a non-fatal error.
	Print the system error message only if the message is non-null. */
{
	if (lstrlen(UserMessage) > 0)
		ReportError(UserMessage, 0, TRUE);
	/* If fatal, raise an exception. */

	if (ExceptionCode != 0)
		RaiseException(
			(0x0FFFFFFF & ExceptionCode) | 0xE0000000, 0, 0, NULL);

	return;
}



BOOL PrintStrings(HANDLE hOut, ...)

/* Write the messages to the output handle. Frequently hOut
	will be standard out or error, but this is not required.
	Use WriteConsole (to handle Unicode) first, as the
	output will normally be the console. If that fails, use WriteFile.

	hOut:	Handle for output file.
	... :	Variable argument list containing TCHAR strings.
		The list must be terminated with NULL. */
{
	DWORD MsgLen, Count;
	LPCTSTR pMsg;
	va_list pMsgList;	/* Current message string. */
	va_start(pMsgList, hOut);	/* Start processing msgs. */
	while ((pMsg = va_arg(pMsgList, LPCTSTR)) != NULL) {
		MsgLen = lstrlen(pMsg);
		if (!WriteConsole(hOut, pMsg, MsgLen, &Count, NULL)
			&& !WriteFile(hOut, pMsg, MsgLen * sizeof(TCHAR),
				&Count, NULL))
			return FALSE;
	}
	va_end(pMsgList);
	return TRUE;
}


BOOL PrintMsg(HANDLE hOut, LPCTSTR pMsg)

/* For convenience only - Single message version of PrintStrings so that
	you do not have to remember the NULL arg list terminator.

	hOut:	Handle of output file
	pMsg:	Single message to output. */
{
	return PrintStrings(hOut, pMsg, NULL);
}