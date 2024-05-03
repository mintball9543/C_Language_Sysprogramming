#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include<wchar.h>
#include<string.h>
#include<stdarg.h>
#include<io.h>
#define STRING_SIZE 256

typedef struct _RECORD {
	DWORD referenceCount;
	SYSTEMTIME recordCreationTime;
	SYSTEMTIME recordLastReferenceTime;
	SYSTEMTIME recordUpdateTime;
	TCHAR dataString[STRING_SIZE];
}RECORD;
typedef struct {
	DWORD nmRecords;
	DWORD numNonEmptyRecords;
}HEADER;

int _tmain(int argc, char* argv[]) {
	HANDLE hFile;
	LARGE_INTEGER curPtr;
	DWORD openOption, nXfer, recNo;
	RECORD record;
	TCHAR string[STRING_SIZE], command, extra;
	OVERLAPPED ov = { 0,0,0,0,NULL }, ovZero = { 0,0,0,0,NULL };
	HEADER header = { 0,0 };
	SYSTEMTIME currentTime;
	BOOLEAN headerChange, recordChange;

	openOption = ((argc > 2 && atoi(argv[2]) <= 0) || argc <= 2) ? OPEN_EXISTING : CREATE_ALWAYS;
	hFile = CreateFile(argv[1], GENERIC_READ|GENERIC_WRITE,0,NULL,openOption,FILE_FLAG_RANDOM_ACCESS,NULL);
	if (argc >= 3 && atoi(argv[2]) > 0) {
		header.nmRecords = atoi(argv[2]);
		WriteFile(hFile, &header, sizeof(header), &nXfer, &ovZero);
		curPtr.QuadPart = (LONGLONG)sizeof(RECORD) * atoi(argv[2]) + sizeof(HEADER);
		//printf("%d-%d-%d", sizeof(RECORD), atoi(argv[2]), sizeof(HEADER));
		SetFilePointerEx(hFile, curPtr, NULL, FILE_BEGIN);
		SetEndOfFile(hFile);
		return 0;
	}

	ReadFile(hFile, &header, sizeof(HEADER), &nXfer, &ovZero);

	while (TRUE) {
		headerChange = FALSE; recordChange = FALSE;
		_tprintf(_T("Enter r(ead)/w(rite)/d(elete)/qu(it) Rec#\n"));
		_tscanf(_T("%c%u%c"), &command, &recNo, &extra);
		if (command == 'q') break;
		if (recNo >= header.nmRecords) {
			_tprintf(_T("Record Number is too large. Try again.\n"));
			continue;
		}
		curPtr.QuadPart = (LONGLONG)recNo * sizeof(RECORD) + sizeof(HEADER);
		ov.Offset = curPtr.LowPart;
		ov.OffsetHigh = curPtr.HighPart;
		ReadFile(hFile, &record, sizeof(RECORD), &nXfer, &ov);
		GetSystemTime(&currentTime);
		record.recordLastReferenceTime = currentTime;
		if (command == 'r' || command == 'd') {
			if (record.referenceCount == 0) {
				_tprintf(_T("Record Number %d is empty.\n"), recNo);
				continue;
			}
			else {
				_tprintf(_T("Record Number %d. Reference Count: %d\n"), recNo, record.referenceCount);
				_tprintf(_T("Data: %s\n"), record.dataString);
			}
			if (command == 'd') {
				record.referenceCount = 0;
				header.numNonEmptyRecords--;
				headerChange = TRUE;
				recordChange = TRUE;
			}
		}
		else if (command == 'w') {
			_tprintf(_T("Enter new data string for the record.\n"));
			_fgetts(string, sizeof(string), stdin);
			string[_tcslen(string) - 1] = _T('\0');
			if (record.referenceCount == 0) {
				record.recordCreationTime = currentTime;
				header.numNonEmptyRecords++;
				headerChange = TRUE;
			}
			record.recordUpdateTime = currentTime;
			record.referenceCount++;
			strncpy(record.dataString, string, STRING_SIZE - 1);
			recordChange = TRUE;
		}
		else {
			_tprintf(_T("Command must be r, w, or d. Try again.\n"));
		}
		if (recordChange)
			WriteFile(hFile, &record, sizeof(RECORD), &nXfer, &ov);
		if (headerChange)
			WriteFile(hFile, &header, sizeof(header), &nXfer, &ovZero);
	}
	_tprintf(_T("Computed number of non-empty records is: %d\n"), header.numNonEmptyRecords);
	ReadFile(hFile, &header, sizeof(HEADER), &nXfer, &ovZero);
	_tprintf(_T("File %s has %d non-empty records.\nCapacity: %d\n"), argv[1], header.numNonEmptyRecords, header.nmRecords);

	CloseHandle(hFile);
	return 0;

}