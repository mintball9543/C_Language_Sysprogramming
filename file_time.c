#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include<wchar.h>

DWORD Options(int argc, LPCTSTR argv[], LPCTSTR OptStr, ...);

int _tmain(int argc, LPTSTR argv[]) {
	FILETIME newFileTime;
	LPFILETIME pAccessTime = NULL, pModifyTime = NULL;
	HANDLE hFile;
	BOOL setAccessTime, setModTime, NotCreateNew, maFlag;
	DWORD createFlag;
	int i, fileIndex;
	fileIndex = Options(argc, argv, _T("amc"), &setAccessTime, &setModTime, &NotCreateNew, NULL);

	maFlag = setAccessTime || setModTime;
	createFlag = NotCreateNew ? OPEN_EXISTING : OPEN_ALWAYS;

	for (i = fileIndex; i < argc; i++) {
		hFile = CreateFile(argv[i], GENERIC_READ | GENERIC_WRITE, 0, NULL, createFlag, FILE_ATTRIBUTE_NORMAL, NULL);
		GetSystemTimeAsFileTime(&newFileTime);
		if (setAccessTime || !maFlag)pAccessTime = &newFileTime;
		if (setModTime || !maFlag)pModifyTime = &newFileTime;
		SetFileTime(hFile, NULL, pAccessTime, pModifyTime);
		CloseHandle(hFile);
	}
	return 0;
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