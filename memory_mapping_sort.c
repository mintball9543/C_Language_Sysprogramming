#include<Windows.h>
#include<stdio.h>
#include<tchar.h>
#include<io.h>

#define TSIZE sizeof (TCHAR)
#define DATALEN 56
#define KEY_SIZE 8

int KeyCompare(LPCTSTR pKey1, LPCTSTR pKey2);

typedef struct _RECORD {
	TCHAR key[KEY_SIZE];
	TCHAR data[DATALEN];
} RECORD;

#define RECSIZE sizeof (RECORD)


int _tmain(int argc, LPCTSTR argv[]) {
	HANDLE hFile = INVALID_HANDLE_VALUE, hMap = NULL;
	LPVOID pFile = NULL;
	LARGE_INTEGER filesize;
	TCHAR tempFile[MAX_PATH];
	LPTSTR pTFile;

	_stprintf(tempFile, _T("%s.tmp"), argv[1]);
	CopyFile(argv[1], tempFile, TRUE);

	hFile = CreateFile(tempFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	GetFileSizeEx(hFile, &filesize);
	filesize.QuadPart += 2;
	hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, filesize.HighPart, filesize.LowPart, NULL);
	pFile = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	qsort(pFile, filesize.QuadPart / RECSIZE, RECSIZE, KeyCompare);

	pTFile = (LPTSTR)pFile;
	pTFile[filesize.QuadPart / TSIZE] = '\0';
	_tprintf(_T("%s"), pFile);
	UnmapViewOfFile(pFile);
	CloseHandle(hMap);
	CloseHandle(hFile);
	DeleteFile(tempFile);
	return 0;
}


int KeyCompare(LPCTSTR pKey1, LPCTSTR pKey2)

/* Compare two records of generic characters.
The key position and length are global variables. */
{
	return _tcsncmp(pKey1, pKey2, KEY_SIZE);
}

