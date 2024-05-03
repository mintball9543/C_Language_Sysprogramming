#include<Windows.h>
#include<stdio.h>
#include<tchar.h>
#include<string.h>

int compare(const char* a, const char* b);

int main(int argc, char* argv[]) {

	HANDLE file;
	HANDLE hNode, hData;
	char buffer[1024] = { 0 };
	int read_size = 0;
	char* ptr[126] = { NULL, };
	int temp;
	int j;
	file = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	hNode = HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_NO_SERIALIZE, 0, 0);

	for (j = 0; j < 126; j++) {
		//ptr[j] = HeapAlloc(hNode, HEAP_ZERO_MEMORY, 1024);
		ptr[j] = malloc(sizeof(char) * 126);
		memset(ptr[j], 0, 126);
	}
	ReadFile(file, buffer, 1024, &read_size, 0);
	//strcpy(ptr, buffer);
	j = 0;
	ptr[j] = strtok(buffer, "\n");

	while (ptr[j] != NULL) {
		ptr[j+1] = strtok(NULL, "\n");
		j++;
	}

	qsort((char *)ptr, j-1, sizeof(char *), compare);
	//temp=strcmp(ptr[0], ptr[1]);
	//printf("%d\n", temp);
	j = 0;
	while (ptr[j] != NULL) {
		printf("%s\n", ptr[j]);
		j++;
	}

	//HeapDestroy(hNode);
	return 0;
}

int compare(const void* a, const void* b) {
	
	return strcmp(a, b);

}