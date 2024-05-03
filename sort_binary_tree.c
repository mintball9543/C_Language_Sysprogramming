#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include <stdarg.h>

#ifdef _UNICODE /* This declaration had to be added. */
#define _memtchr wmemchr
#else
#define _memtchr memchr
#endif

DWORD Options(int argc, LPCTSTR argv[], LPCTSTR OptStr, ...);
VOID ReportError(LPCTSTR UserMessage, DWORD ExitCode, BOOL PrintErrorMsg);
VOID ReportException(LPCTSTR UserMessage, DWORD ExceptionCode);
BOOL PrintStrings(HANDLE hOut, ...);
BOOL PrintMsg(HANDLE hOut, LPCTSTR pMsg);

#define CR 0x0D
#define LF 0x0A
#define TSIZE sizeof (TCHAR)

#define KEY_SIZE 8
typedef struct _TREENODE {
	struct _TREENODE* left, * right;
	TCHAR key[KEY_SIZE];
	LPTSTR pData;
}TREENODE, *LPTNODE, **LPPTNODE;
#define NODE_SIZE sizeof (TREENODE)
#define NODE_HEAP_ISIZE 0x8000
#define DATA_HEAP_ISIZE 0x8000
#define MAX_DATA_LEN 0x1000
#define TKEY_SIZE KEY_SIZE *sizeof (TCHAR)
#define STATUS_FILE_ERROR 0xE0000001

LPTNODE FillTree(HANDLE, HANDLE, HANDLE);
BOOL Scan(LPTNODE);
int KeyCompare(LPCTSTR, LPCTSTR), iFile;
BOOL InsertTree(LPPTNODE, LPTNODE);


int _tmain(int argc, LPTSTR argv[]) {
	HANDLE hIn = INVALID_HANDLE_VALUE, hNode = NULL, hData = NULL;
	LPTNODE pRoot;
	BOOL noPrint;
	char errorMessage[256];
	int iFirstFile = Options(argc, argv, _T("n"), &noPrint, NULL);

	for (iFile = iFirstFile; iFile < argc; iFile++) __try {
		hIn = CreateFile(argv[iFile], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hIn == INVALID_HANDLE_VALUE)
			RaiseException(STATUS_FILE_ERROR, 0, 0, NULL);

		__try {
			hNode = HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_NO_SERIALIZE, NODE_HEAP_ISIZE, 0);
			hData = HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_NO_SERIALIZE, DATA_HEAP_ISIZE, 0);

			pRoot = FillTree((HANDLE)hIn, (HANDLE)hNode, (HANDLE)hData);
			_tprintf(_T("Sorted file: %s\n"), argv[iFile]);
			Scan(pRoot);
		}
		__finally {
			if (hNode != NULL) HeapDestroy(hNode);
			if (hNode != NULL) HeapDestroy(hData);
			hNode = NULL; hData = NULL;
			if (hIn != INVALID_HANDLE_VALUE) CloseHandle(hIn);
			hIn = INVALID_HANDLE_VALUE;
		}
	}

	__except ((GetExceptionCode() == STATUS_FILE_ERROR || GetExceptionCode() == STATUS_NO_MEMORY) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
		_stprintf(errorMessage, _T("\n%s %s"), _T("SortBT error on file:"), argv[iFile]);
		ReportError(errorMessage, 0, TRUE);
	}
	return 0;
}

LPTNODE FillTree(HANDLE hIn, HANDLE hNode, HANDLE hData) {
	LPTNODE pRoot = NULL, pNode;
	DWORD nRead, i;
	BOOL atCR;
	char dataHold[MAX_DATA_LEN];
	LPTSTR pString;

	while (TRUE) {
		pNode = HeapAlloc(hNode, HEAP_ZERO_MEMORY, NODE_SIZE);
		if (!ReadFile(hIn, pNode->key, TKEY_SIZE, &nRead, NULL) || nRead != TKEY_SIZE)
			return pRoot;

		atCR = FALSE;
		for (i = 0; i < MAX_DATA_LEN; i++) {
			ReadFile(hIn, &dataHold[i], TSIZE, &nRead, NULL);
			if (atCR && dataHold[i] == LF) break;
			atCR = (dataHold[i] == CR);
		}

		dataHold[i - 1] = '\0';

		pString = HeapAlloc(hData, HEAP_ZERO_MEMORY, (SIZE_T)(KEY_SIZE + _tcslen(dataHold) + 1) * TSIZE);
		memcpy(pString, pNode->key, TKEY_SIZE);
		pString[KEY_SIZE] = '\0';
		_tcscat(pString, dataHold);
		pNode->pData = pString;
		InsertTree(&pRoot, pNode);
	}
	return NULL;
}

BOOL InsertTree(LPPTNODE ppRoot, LPTNODE pNode) {
	if (*ppRoot == NULL) {
		*ppRoot = pNode;
		return TRUE;
	}
	if (KeyCompare(pNode->key, (*ppRoot)->key) < 0)
		InsertTree(&((*ppRoot)->left), pNode);
	else
		InsertTree(&((*ppRoot)->right), pNode);
}

static int KeyCompare(LPCTSTR pKey1, LPCTSTR pKey2) {
	return _tcsncmp(pKey1, pKey2, KEY_SIZE);
}

static BOOL Scan(LPTNODE pNode) {
	if (pNode == NULL) return TRUE;
	Scan(pNode->left);
	_tprintf(_T("%s\n"), pNode->pData);
	Scan(pNode->right);
	return TRUE;
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
	DWORD msgLen, count;
	LPCTSTR pMsg;
	va_list pMsgList;	/* Current message string. */
	va_start(pMsgList, hOut);	/* Start processing msgs. */
	while ((pMsg = va_arg(pMsgList, LPCTSTR)) != NULL) {
		msgLen = lstrlen(pMsg);
		if (!WriteConsole(hOut, pMsg, msgLen, &count, NULL)
			&& !WriteFile(hOut, pMsg, msgLen * sizeof(TCHAR), &count, NULL)) {
			va_end(pMsgList);
			return FALSE;
		}
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
		for (iArg = 1; !(*pFlag) && iArg < argc && argv[iArg][0] == '-'; iArg++)
			*pFlag = _memtchr(argv[iArg], OptStr[iFlag],
				_tcslen(argv[iArg])) != NULL;
		iFlag++;
	}

	va_end(pFlagList);

	for (iArg = 1; iArg < argc && argv[iArg][0] == '-'; iArg++);

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

/* Extension of ReportError to generate a user exception
	code rather than terminating the process. */

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