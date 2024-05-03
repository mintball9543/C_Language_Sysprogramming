#include<stdio.h>
#include<tchar.h>
#include<wchar.h>
#include<Windows.h>
#include <stdarg.h>

#define TYPE_FILE 1		/* Used in ls, rm, and lsFP */
#define TYPE_DIR 2
#define TYPE_DOT 3

#define MAX_OPTIONS 20	/* Max # command line options */
#define MAX_ARG 1000	/* Max # of command line arguments */
#define MAX_COMMAND_LINE MAX_PATH+50 /* Max size of a command line */

BOOL TraverseDirectory(LPCTSTR, DWORD, LPBOOL);
DWORD FileType(LPWIN32_FIND_DATA);
BOOL ProcessItem(LPWIN32_FIND_DATA, DWORD, LPBOOL);
DWORD Options(int argc, LPCTSTR argv[], LPCTSTR OptStr, ...);

int _tmain(int argc, LPTSTR argv[]) {
	BOOL flags[MAX_OPTIONS], ok = TRUE;
	TCHAR pathName[MAX_PATH + 1], currPath[MAX_PATH + 1], tempPath[MAX_PATH+1];
	LPTSTR pSlash, pFileName;
	int i, fileIndex;

	fileIndex = Options(argc, argv, _T("R1"), &flags[0], &flags[1], NULL);

	GetCurrentDirectory(MAX_PATH, currPath);
	if (argc < fileIndex + 1)
		ok = TraverseDirectory(_T("*"), MAX_OPTIONS, flags);
	else for (i = fileIndex; i < argc; i++) {
		_tcscpy(pathName, argv[i]);
		swprintf(tempPath, MAX_PATH+1,L"%s\\%s1", currPath, pathName);

		pSlash = strrchr(tempPath, '\\');
		if (pSlash != NULL) {
			*pSlash = '\0';
			_tcscat(tempPath, _T("\\"));
			SetCurrentDirectory(tempPath);
			pSlash = strrchr(pathName, '\\');
			pFileName = pSlash + 1;
		}
		else pFileName = pathName;
		ok = TraverseDirectory(tempPath, MAX_OPTIONS, flags) && ok;
		SetCurrentDirectory(currPath);
	}
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
			*pFlag = memchr(argv[iArg], OptStr[iFlag],
				_tcslen(argv[iArg])) != NULL;
		iFlag++;
	}

	va_end(pFlagList);

	for (iArg = 1; iArg < argc && argv[iArg][0] == '-'; iArg++);

	return iArg;
}

static BOOL TraverseDirectory(LPCTSTR pathName, DWORD numFlags, LPBOOL flags) {
	HANDLE searchHandle;
	WIN32_FIND_DATA findData;
	BOOL recursive = flags[0];
	DWORD fType, iPass;
	TCHAR currPath[MAX_PATH + 1];

	GetCurrentDirectory(MAX_PATH, currPath);

	for (iPass = 1; iPass <= 2; iPass++ ) {
		searchHandle = FindFirstFile(pathName, &findData);
		if (INVALID_HANDLE_VALUE == searchHandle)
			printf("%d\n", GetLastError());
		do {
			fType = FileType(&findData);
			if (iPass == 1)
				ProcessItem(&findData, MAX_OPTIONS, flags);
			if (fType == TYPE_DIR && iPass == 2 && recursive) {
				_tprintf(_T("\n%s\\%s:"), currPath, findData.cFileName);
				SetCurrentDirectory(findData.cFileName);
				TraverseDirectory(_T("*"), numFlags, flags);
				SetCurrentDirectory(_T(".."));
			}
		} while (FindNextFile(searchHandle, &findData));
		FindClose(searchHandle);
	}
	return TRUE;
}

static BOOL ProcessItem(LPWIN32_FIND_DATA pFileData, DWORD numFlags, LPBOOL flags) {
	const TCHAR fileTypeChar[] = { ' ','d' };
	DWORD fType = FileType(pFileData);
	BOOL Long = flags[1];
	SYSTEMTIME lastWrite,test;

	if (fType != TYPE_FILE && fType != TYPE_DIR) return FALSE;

	_tprintf(_T("\n"));
	if (Long) {
		_tprintf(_T("%c"), fileTypeChar[fType - 1]);
		_tprintf(_T("%10d"), pFileData->nFileSizeLow);
		FileTimeToLocalFileTime(&(pFileData->ftLastWriteTime), &lastWrite);
		FileTimeToSystemTime(&lastWrite, &test);
		_tprintf(_T("\t%02d/%02d/%04d %02d:%02d:%02d"), test.wMonth, test.wDay, test.wYear, test.wHour, test.wMinute, test.wSecond);

	}
	_tprintf(_T(" %s"), pFileData->cFileName);
	return TRUE;
}

static DWORD FileType(LPWIN32_FIND_DATA pFileData) {
	BOOL isDir;
	DWORD fType;
	fType = TYPE_FILE;
	isDir = (pFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

	if (isDir)
		if (lstrcmp(pFileData->cFileName, _T(".")) == 0 || lstrcmp(pFileData->cFileName, _T("..")) == 0)
			fType = TYPE_DOT;
		else
			fType = TYPE_DIR;
	return fType;
}