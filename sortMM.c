#include<Windows.h>
#include<stdio.h>
#include<tchar.h>
#include<io.h>
#include<stdarg.h>
#include<wchar.h>

#define TSIZE sizeof (TCHAR)
#define CR 0x0D
#define LF 0x0A
#define KEY_SIZE 8
#ifdef _UNICODE /* This declaration had to be added. */
#define _memtchr wmemchr
#else
#define _memtchr memchr
#endif

LPTSTR pInFile = NULL;
int KeyCompare(LPCTSTR, LPCTSTR);
VOID CreateIndexFile(LARGE_INTEGER, LPCTSTR);
DWORD Options(int argc, LPCTSTR argv[], LPCTSTR OptStr, ...);
VOID ReportException(LPCTSTR UserMessage, DWORD ExceptionCode);
VOID ReportError(LPCTSTR UserMessage, DWORD ExitCode, BOOL PrintErrorMsg);
int KeyCompare(LPCTSTR pKey1, LPCTSTR pKey2);

DWORD_PTR kStart = 0, KSize = 8;
BOOL reverse;

int _tmain(int argc, LPCTSTR argv[]) {
	HANDLE hInFile, hInMap;
	HANDLE hXFile, hXMap;
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	BOOL idxExists, noPrint;
	DWORD indexSize, rXSize, iKey, nWrite;
	LARGE_INTEGER inputSize;
	
	LPBYTE pXFile = NULL, px;
	TCHAR __based (pInFile)*pIn, indexFileName[MAX_PATH], chNewLine = _T('\n');
	int FlIdx = Options(argc, argv, _T("rIn"), &reverse, &idxExists, &noPrint, NULL);

	hInFile = CreateFile(argv[FlIdx], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	hInMap = CreateFileMapping(hInFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	pInFile = MapViewOfFile(hInMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (!GetFileSizeEx(hInFile, &inputSize))
		ReportError(_T("Failed to get input file size."), 5, TRUE);

	_stprintf(indexFileName, _T("%s.idx"), argv[FlIdx]);
	if (!idxExists)
		CreateIndexFile(inputSize, indexFileName);

	hXFile = CreateFile(indexFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	hXMap = CreateFileMapping(hXFile, NULL, PAGE_READWRITE, 0, 0, 0);
	pXFile = MapViewOfFile(hXMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	indexSize = GetFileSize(hXFile, NULL);

	rXSize = KSize * TSIZE + sizeof(LPTSTR);

	if (!idxExists)
		qsort(pXFile, indexSize / rXSize, rXSize, KeyCompare);

	px = pXFile + rXSize - sizeof(LPTSTR);
	if (!noPrint) {
		for (iKey = 0; iKey < indexSize / rXSize; iKey++) {
			WriteFile(hStdOut, &chNewLine, TSIZE, &nWrite, NULL);
			pIn = (TCHAR __based (pInFile)*) * (DWORD*)px;
			while ((SIZE_T)pIn < inputSize.QuadPart && (*pIn != CR || *(pIn + 1) != LF)) {
				WriteFile(hStdOut, pIn, TSIZE, &nWrite, NULL);
				pIn++;
			}
			px += rXSize;
		}
		UnmapViewOfFile(pInFile);
		CloseHandle(hInMap); CloseHandle(hInFile);
		UnmapViewOfFile(pXFile);
		CloseHandle(hXMap); CloseHandle(hXFile);
		return 0;
	}
}

VOID CreateIndexFile(LARGE_INTEGER inputSize, LPCTSTR indexFileName) {
	HANDLE hXFile;
	TCHAR __based (pInFile)*pInScan = 0;
	DWORD nWrite;

	hXFile = CreateFile(indexFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);

	while ((DWORD_PTR)pInScan < inputSize.QuadPart) {
		WriteFile(hXFile, pInScan + kStart, KSize * TSIZE, &nWrite, NULL);
		WriteFile(hXFile, &pInScan, sizeof(LPTSTR), &nWrite, NULL);
		while ((DWORD) (DWORD_PTR)pInScan < inputSize.QuadPart - sizeof(TCHAR) && ((*pInScan != CR) || (*(pInScan + 1) != LF))) {
			pInScan++;
		}
		pInScan += 2;
	}
	CloseHandle(hXFile);
	return;
}

int KeyCompare(LPCTSTR pKey1, LPCTSTR pKey2)

/* Compare two records of generic characters.
The key position and length are global variables. */
{
	return _tcsncmp(pKey1, pKey2, KEY_SIZE);
}



DWORD Options(int argc, LPCTSTR argv[], LPCTSTR OptStr, ...)

/* argv is the command line.
	The options, if any, start with a '-' in argv[1], argv[2], ...
	OptStr is a text string containing all possible options,
	in one-to-one correspondence with the addresses of Boolean variables
	in the variable argument list (...).
	These flags are set if and only if the corresponding option
	character occurs in argv [1], argv [2], ...
	The return value is the argv index of the first argument beyond the options. */

{
	va_list pFlagList;
	LPBOOL pFlag;
	int iFlag = 0, iArg;

	va_start(pFlagList, OptStr);

	while ((pFlag = va_arg(pFlagList, LPBOOL)) != NULL
		&& iFlag < (int)_tcslen(OptStr)) {
		*pFlag = FALSE;
		for (iArg = 1; !(*pFlag) && iArg < argc && argv[iArg][0] == _T('-'); iArg++)
			*pFlag = _memtchr(argv[iArg], OptStr[iFlag],
				_tcslen(argv[iArg])) != NULL;
		iFlag++;
	}

	va_end(pFlagList);

	for (iArg = 1; iArg < argc && argv[iArg][0] == _T('-'); iArg++);

	return iArg;
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