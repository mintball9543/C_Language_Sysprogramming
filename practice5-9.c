#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include<string.h>

int main(int argc, char* argv[]) {
	HANDLE hFile,hinmap;
	char* pFile;
	char* buffer[126] = { 0 };
	char* temp;
	int idx = 0;
	int line = 0;
	int i = 0;

	hFile = CreateFile(argv[1], GENERIC_ALL, 0, NULL, OPEN_EXISTING, 0, NULL);
	hinmap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	pFile = MapViewOfFile(hinmap, FILE_MAP_READ, 0, 0, 0);
	
	
	temp = malloc(sizeof(char) * 1024);
	strcpy(temp, pFile);
	
	buffer[idx]=strtok(temp, "\n");
	if (buffer[idx] == NULL) return 0;
	else idx++;

	while (buffer[idx-1]!=NULL) {
		buffer[idx]=strtok(NULL, "\n");
		idx++;
	}

	printf("%d줄이 있습니다. 마지막으로부터 몇 줄을 출력하시겠습니까?: ", idx);
	scanf("%d", &line);

	for (idx-=2; line > 0; line--, idx--) {
		printf("%s\n", buffer[idx]);
	}

	return 0;
}