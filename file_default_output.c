#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include<wchar.h>
#include<string.h>
#include<stdarg.h>
#define BUF_SIZE 0x200

static void catfile(HANDLE, HANDLE);
DWORD Options(int argc, LPCTSTR argv[], LPCTSTR OptStr, ...);

int _tmain(int argc, LPTSTR argv[]) {
	HANDLE hinfile, hstdin = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
	BOOL dashs;
	int iarg, ifirstfile;
	ifirstfile = Options(argc, argv, _T("s"), &dashs, NULL);
	if (ifirstfile == argc) {
		catfile(hstdin, hstdout);
		return 0;
	}

	for (iarg = ifirstfile; iarg < argc; iarg++) {
		hinfile = CreateFile(argv[iarg], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hinfile == INVALID_HANDLE_VALUE) {
			//if (!dashs) reporterror(_T("Error: File does not exist."), 0, TRUE);
		}
		else {
			catfile(hinfile, hstdout);
			//if (GetLastError() != 0 && !dashs)
				//reporterror(_T("Cat Error."), 0, TRUE);
			CloseHandle(hinfile);
		}
	}
	return 0;
}

DWORD Options(int argc, LPCTSTR argv[], LPCTSTR OptStr, ...) {
	va_list pFlagList;
	LPBOOL pFlag;
	int iFlag = 0, iArg;

	va_start(pFlagList, OptStr);

	while ((pFlag = va_arg(pFlagList, LPBOOL)) != NULL
		&& iFlag < (int)_tcslen(OptStr)) {
		*pFlag = FALSE;
		for (iArg = 1; !(*pFlag) && iArg < argc && argv[iArg][0] == '-'; iArg++)
			*pFlag = memchr(argv[iArg], OptStr[iFlag], _tcslen(argv[iArg])) != NULL;
		iFlag++;
	}

	va_end(pFlagList);

	for (iArg = 1; iArg < argc && argv[iArg][0] == '-'; iArg++);

	return iArg;
}

static void catfile(HANDLE hinfile, HANDLE houtfile) {
	DWORD nin, nout;
	BYTE buffer[BUF_SIZE];
	while (ReadFile(hinfile, buffer, BUF_SIZE, &nin, NULL) && (nin != 0) && WriteFile(houtfile, buffer, nin, &nout, NULL));
	return;
}