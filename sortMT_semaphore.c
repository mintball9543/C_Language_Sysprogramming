#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include <stdarg.h>
#include"support.h"

#define DATALEN 56
#define KEYLEN 8
typedef struct _RECORD {
	CHAR key[KEYLEN];
	char data[DATALEN];
}RECORD;
#define RECSIZE sizeof(RECORD)
typedef RECORD* LPRECORD;

typedef struct _THREADARG {
	DWORD iTh;
	LPRECORD lowRecord;
	LPRECORD highRecord;
}THREADARG, *PTHREADARG;

DWORD Options(int argc, LPCTSTR argv[], LPCTSTR OptStr, ...);
int KeyCompare(LPCTSTR pRec1, LPCTSTR pRec2);
static int KeyCompare(LPCTSTR, LPCTSTR);
static DWORD WINAPI SortThread(PTHREADARG pThArg);
static DWORD nRec;
static HANDLE* pThreadHandle;
HANDLE se;
int main(int argc, LPSTR argv[]) {
	HANDLE hFile, mHandle;
	LPRECORD pRecords = NULL;
	DWORD lowRecordNum, nRecTh, numFiles, iTh;
	LARGE_INTEGER fileSize;
	BOOL noPrint;
	int iFF, iNP;
	PTHREADARG threadArg;
	LPTSTR stringEnd;


	iNP = Options(argc, argv, _T("n"), &noPrint, NULL);
	iFF = iNP + 1;
	numFiles = _ttoi(argv[iNP]);
	se = CreateSemaphore(NULL, numFiles, numFiles, NULL);

	hFile = CreateFile(argv[iFF], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	SetFilePointer(hFile, 2, 0, FILE_END);
	SetEndOfFile(hFile);
	mHandle = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);

	GetFileSizeEx(hFile, &fileSize);
	nRec = fileSize.QuadPart / RECSIZE;
	nRecTh = nRec / numFiles;
	threadArg = malloc(numFiles * sizeof(THREADARG));
	pThreadHandle = malloc(numFiles * sizeof(HANDLE));

	pRecords = MapViewOfFile(mHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	CloseHandle(mHandle);

	lowRecordNum = 0;
	for (iTh = 0; iTh < numFiles; iTh++) {
		threadArg[iTh].iTh = iTh;
		threadArg[iTh].lowRecord = pRecords + lowRecordNum;
		threadArg[iTh].highRecord = pRecords + (lowRecordNum + nRecTh);
		lowRecordNum += nRecTh;
		pThreadHandle[iTh] = (HANDLE)_beginthreadex(NULL, 0, SortThread, &threadArg[iTh], CREATE_SUSPENDED, NULL);
	}

	for (iTh = 0; iTh < numFiles; iTh++)
		ResumeThread(pThreadHandle[iTh]);

	WaitForSingleObject(pThreadHandle[0], INFINITE);
	for (iTh = 0; iTh < numFiles; iTh++)
		CloseHandle(pThreadHandle[iTh]);

	stringEnd = (LPTSTR)pRecords + nRec * RECSIZE;
	*stringEnd = _T('\0');
	if (!noPrint) {
		_tprintf(_T("%s"), (LPCTSTR)pRecords);
	}
	UnmapViewOfFile(pRecords);
	SetFilePointer(hFile, -2, 0, FILE_END);
	SetEndOfFile(hFile);

	CloseHandle(hFile);
	free(threadArg);
	free(pThreadHandle);
	return 0;
}

static VOID MergeArrays(LPRECORD, DWORD);

DWORD WINAPI SortThread(PTHREADARG pThArg) {
	DWORD groupSize = 2, myNumber, twoToI = 1;
	DWORD_PTR numbersInGroup;
	LPRECORD first;
	DWORD result;
	myNumber = pThArg->iTh;
	first = pThArg->lowRecord;
	numbersInGroup = pThArg->highRecord - first;

	qsort(first, numbersInGroup,RECSIZE, KeyCompare);

	while ((myNumber % groupSize) == 0 && numbersInGroup < nRec) {
		//WaitForSingleObject(pThreadHandle[myNumber + twoToI], INFINITE);
		result=WaitForSingleObject(se, 5);
		switch (result) {
		case WAIT_OBJECT_0:
			MergeArrays(first, numbersInGroup);
			numbersInGroup *= 2;
			groupSize *= 2;
			twoToI *= 2;
			ReleaseSemaphore(se, 1, NULL);
			break;
		}
	}

	return 0;
}

static VOID MergeArrays(LPRECORD p1, DWORD nRecs) {
	DWORD iRec = 0, i1 = 0, i2 = 0;
	LPRECORD pDest, p1Hold, pDestHold, p2 = p1 + nRecs;

	pDest = pDestHold = malloc(2 * nRecs * RECSIZE);
	p1Hold = p1;

	while (i1 < nRecs && i2 < nRecs) {
		if (KeyCompare((LPCTSTR)p1, (LPCTSTR)p2) <= 0) {
			memcpy(pDest, p1, RECSIZE);
			i1++; p1++; pDest++;
		}
		else {
			memcpy(pDest, p2, RECSIZE);
			i2++; p2++; pDest++;
		}
	}
	if (i1 >= nRecs)
		memcpy(pDest, p2, RECSIZE * (nRecs - i2));
	else
		memcpy(pDest, p1, RECSIZE * (nRecs - i1));
	memcpy(p1Hold, pDestHold, 2 * nRecs * RECSIZE);
	free(pDestHold);
	return;
}

/* Utility function to extract option flags from the command line.  */


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

int KeyCompare(LPCTSTR pRec1, LPCTSTR pRec2)
{
	DWORD i;
	TCHAR b1, b2;
	LPRECORD p1, p2;
	int Result = 0;

	p1 = (LPRECORD)pRec1;
	p2 = (LPRECORD)pRec2;
	for (i = 0; i < KEYLEN && Result == 0; i++) {
		b1 = p1->key[i];
		b2 = p2->key[i];
		if (b1 < b2) Result = -1;
		if (b1 > b2) Result = +1;
	}
	return  Result;
}