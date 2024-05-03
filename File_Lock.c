#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include<wchar.h>

int _tmain(int argc, LPTSTR argv[]) {
	HANDLE hFile;
	DWORD filesize;
	WCHAR temp[126];
	OVERLAPPED ov;

	hFile = CreateFile(argv[1], GENERIC_ALL, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		printf("%d",GetLastError());

	GetFileSize(hFile, &filesize);
	LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, filesize, 0, &ov);

	printf("파일 락");
	_getws_s(temp, 126);
	

	UnlockFileEx(hFile, 0, filesize, 0, &ov);
	CloseHandle(hFile);
	printf("파일 언락");

	_getws_s(temp, 126);
	return 0;
}