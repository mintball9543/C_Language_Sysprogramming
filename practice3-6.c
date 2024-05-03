#include<stdio.h>
#include<Windows.h>
#include<stdbool.h>
#include<tchar.h>
#include<wchar.h>

int main(int argc, char* argv[]) {
	WIN32_FIND_DATA filedata;
	HANDLE handle,handle2;
	char filename[126] = { 0 }, subfilename[126] = { 0 };
	char currpath[126] = { 0 };
	SYSTEMTIME time, localtime;

	GetCurrentDirectory(MAX_PATH + 1, currpath);

	sprintf(filename, "%s\\%s", currpath, argv[1]);
	handle=FindFirstFile(filename, &filedata);
	if (GetLastError() == INVALID_HANDLE_VALUE)
		printf("%d", GetLastError());
	else {
		do {
			FileTimeToLocalFileTime(&(filedata.ftCreationTime), &time); //생성 시간
			FileTimeToSystemTime(&time, &localtime);

			printf("생성: %04d/%02d/%02d %02d:%02d:%02d\t", localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);
			
			FileTimeToLocalFileTime(&(filedata.ftLastAccessTime), &time); //마지막 접근 시간
			FileTimeToSystemTime(&time, &localtime);

			printf("접근: %04d/%02d/%02d %02d:%02d:%02d\t", localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);

			FileTimeToLocalFileTime(&(filedata.ftLastWriteTime), &time); //마지막 수정 시간
			FileTimeToSystemTime(&time, &localtime);
			
			printf("수정: %04d/%02d/%02d %02d:%02d:%02d\t", localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);
			
			
			printf("%d\t%s\n", filedata.nFileSizeLow, filedata.cFileName);

			
			//하위 파일들			
			GetCurrentDirectory(126, subfilename);
			sprintf(subfilename,"%s\\%s\\%s", subfilename,filedata.cFileName,"*");
			handle2 = FindFirstFile(subfilename, &filedata);
			if (handle2 != INVALID_HANDLE_VALUE) {
				do {
					FileTimeToLocalFileTime(&(filedata.ftCreationTime), &time); //생성 시간
					FileTimeToSystemTime(&time, &localtime);

					printf("\t생성: %04d/%02d/%02d %02d:%02d:%02d\t", localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);

					FileTimeToLocalFileTime(&(filedata.ftLastAccessTime), &time); //마지막 접근 시간
					FileTimeToSystemTime(&time, &localtime);

					printf("접근: %04d/%02d/%02d %02d:%02d:%02d\t", localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);

					FileTimeToLocalFileTime(&(filedata.ftLastWriteTime), &time); //마지막 수정 시간
					FileTimeToSystemTime(&time, &localtime);

					printf("수정: %04d/%02d/%02d %02d:%02d:%02d\t", localtime.wYear, localtime.wMonth, localtime.wDay, localtime.wHour, localtime.wMinute, localtime.wSecond);
					printf("%d\t%s\n", filedata.nFileSizeLow, filedata.cFileName);
				} while (FindNextFile(handle2, &filedata));
			}
		
		
		} while (FindNextFile(handle, &filedata));
	}
	return 0;
}