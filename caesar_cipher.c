#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include<wchar.h>
#include<string.h>
#include<stdarg.h>
#include<io.h>

BOOL cci_f(LPCTSTR, LPCTSTR, DWORD);

int _tmain(int argc, LPTSTR argv[]) {
	
	cci_f(argv[2], argv[3], atoi(argv[1]));
	return 0;
}

BOOL cci_f(LPCTSTR fIn, LPCTSTR fOut, DWORD shift) {
	HANDLE hIn, hOut;
	DWORD nIn, nOut, iCopy;
	char abuffer[16384],ccbuffer[16384];
	BOOL writeOK = TRUE;

	hIn = CreateFile(fIn, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) return FALSE;
	hOut = CreateFile(fOut, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) return FALSE;

	while (ReadFile(hIn, abuffer, 16384, &nIn, NULL) && nIn > 0 && writeOK) {
		for (iCopy = 0; iCopy < nIn; iCopy++)
			ccbuffer[iCopy] = (abuffer[iCopy] + shift) % 256;
		writeOK = WriteFile(hOut, ccbuffer, nIn, &nOut, NULL);
	}

	CloseHandle(hIn);
	CloseHandle(hOut);

	return writeOK;
}