#include<stdio.h>
#include<Windows.h>
#include<wchar.h>
#include<tchar.h>
#include<stdarg.h>

VOID ReportError(LPCTSTR UserMessage, DWORD ExitCode, BOOL PrintErrorMsg);
VOID ReportException(LPCTSTR UserMessage, DWORD ExceptionCode);
BOOL PrintStrings(HANDLE hOut, ...);
BOOL PrintMsg(HANDLE hOut, LPCTSTR pMsg);

int _tmain(DWORD argc, LPTSTR argv[]) {
	HANDLE hIn = INVALID_HANDLE_VALUE, hOut = INVALID_HANDLE_VALUE;
	DWORD nXfer, iFile, j;
	char outFileName[256] = "", * pBuffer = NULL;
	OVERLAPPED ov = { 0,0,0,0,NULL };
	LARGE_INTEGER fSize;

	for (iFile = 1; iFile < argc; iFile++) __try {
		if (_tcslen(argv[iFile]) > 250)
			ReportException(_T("The file name is too long."), 1);
		_stprintf(outFileName, _T("UC_%s"), argv[iFile]);

		__try {
			hIn = CreateFile(argv[iFile], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
			if (hIn == INVALID_HANDLE_VALUE)
				ReportException(argv[iFile], 1);

			if (!GetFileSizeEx(hIn, &fSize) || fSize.HighPart > 0)
				ReportException(_T("This file is too large."), 1);

			hOut = CreateFile(outFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL);
			if (hOut == INVALID_HANDLE_VALUE)
				ReportException(outFileName, 1);

			pBuffer = malloc(fSize.LowPart);
			if (pBuffer == NULL)
				ReportException(_T("Memory allocation error"), 1);

			if (!ReadFile(hIn, pBuffer, fSize.LowPart, &nXfer, NULL) || (nXfer != fSize.LowPart))
				ReportException(_T("ReadFile error"), 1);

			for (j = 0; j < fSize.LowPart; j++)
				if (isalpha(pBuffer[j])) pBuffer[j] = toupper(pBuffer[j]);

			if (!WriteFile(hOut, pBuffer, fSize.LowPart, &nXfer, NULL) || (nXfer != fSize.LowPart))
				ReportException(_T("WriteFile error"), 1);
		}
		__finally {
			if (pBuffer != NULL) free(pBuffer); pBuffer = NULL;
			if (hIn != INVALID_HANDLE_VALUE) {
				CloseHandle(hIn);
				hIn = INVALID_HANDLE_VALUE;
			}
			if (hOut != INVALID_HANDLE_VALUE) {
				CloseHandle(hOut);
				hOut = INVALID_HANDLE_VALUE;
			}
			_tcscpy(outFileName, _T(""));
		}
	}

	__except (EXCEPTION_EXECUTE_HANDLER) {
		_tprintf(_T("Error processing file %s\n"), argv[iFile]);
		DeleteFile(outFileName);
	}
	_tprintf(_T("All files converted, except as noted above\n"));
	return 0;
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