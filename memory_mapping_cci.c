#include<Windows.h>
#include<stdio.h>
#include<tchar.h>
#include<io.h>

VOID ReportError(LPCTSTR UserMessage, DWORD ExitCode, BOOL PrintErrorMsg);
VOID ReportException(LPCTSTR UserMessage, DWORD ExceptionCode);


BOOL cci_f(LPCTSTR fIn, LPCTSTR fOut, DWORD shift) {
	BOOL complete = FALSE;
	__try {
		HANDLE hIn, hOut;
		HANDLE hInMap, hOutMap;
		LPTSTR pIn, pInFile, pOut, pOutFile;
		LARGE_INTEGER filesize;

		hIn = CreateFile(fIn, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		GetFileSizeEx(hIn, &filesize);
		if (filesize.HighPart > 0 && sizeof(SIZE_T) == 4)
			ReportException(_T("File is too large for Win32."), 4);

		hInMap = CreateFileMapping(hIn, NULL, PAGE_READONLY, 0, 0, NULL);
		pInFile = MapViewOfFile(hInMap, FILE_MAP_READ, 0, 0, 0);

		hOut = CreateFile(fOut, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		hOutMap = CreateFileMapping(hOut, NULL, PAGE_READWRITE, filesize.HighPart, filesize.LowPart, NULL);
		pOutFile = MapViewOfFile(hOutMap, FILE_MAP_WRITE, 0, 0, (SIZE_T)filesize.QuadPart);

		__try {
			pIn = pInFile;
			pOut = pOutFile;
			while (pIn < pInFile + filesize.QuadPart) {
				*pOut = (*pIn + shift) % 256;
				pIn++; pOut++;
			}
			complete = TRUE;
		}
		__except (GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
			complete = FALSE;
			ReportError(_T("Fatal Error accessing mapped file."), 9, TRUE);
		}

		UnmapViewOfFile(pOutFile);
		UnmapViewOfFile(pInFile);
		CloseHandle(hOutMap);
		CloseHandle(hInMap);
		CloseHandle(hIn);
		CloseHandle(hOut);
		return complete;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		if (!complete)
			DeleteFile(fOut);
		return FALSE;
	}
}

int _tmain(int argc, LPTSTR argv[]) {
	if(argc!=4)
		ReportError(_T("Usage: cci shift file1 file2"), 1, FALSE);

	if (!cci_f(argv[2], argv[3], atoi(argv[1])))
		ReportError(_T("Encryption failed."), 4, TRUE);

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