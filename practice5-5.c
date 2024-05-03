#include<stdio.h>
#include<Windows.h>
#include<tchar.h>

int _tmain(int argc, char* argv[]) {
	HANDLE hFile,hFileMap;
	char* pFile;
	char* buffer = { 0 };
	int read_size, write_size;
	int menu = 0;


	hFile = CreateFile(argv[1], GENERIC_ALL, 0, NULL, OPEN_ALWAYS, 0, NULL);
	hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	pFile = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	while (1) {
		printf("1.읽기 2.쓰기 3.종료: ");
		scanf("%d", &menu);

		switch (menu) {
		case 1:
			//buffer = malloc(sizeof(char) * 1024);
			//memset(buffer, 0, 1024);
			//ReadFile(pFile, buffer, 1024, &read_size, 0);
			printf("%s\n", pFile);

			break;
		case 2:
			buffer=malloc(sizeof(char) * 126);
			memset(buffer, 0, 126);
			while (getchar() != '\n');
			gets_s(buffer, 126);
			sprintf(pFile, "%s\n%s", pFile, buffer);
			WriteFile(hFile, pFile, strlen(pFile), &write_size, NULL);
			break;
		case 3:
			UnmapViewOfFile(pFile);
			CloseHandle(hFileMap);
			CloseHandle(hFile);
			break;
		}
		if (menu == 3) break;
	}


	return 0;
}