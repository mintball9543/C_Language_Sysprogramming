#include<stdio.h>
#include<Windows.h>
#include<stdbool.h>
#include<tchar.h>
#include<wchar.h>

int main(int argc, char* argv[]) {
	char path[126] = { 0 };
	int select;
	SYSTEMTIME time;
	WIN32_FIND_DATA filedata;
	HANDLE handle;
	int temp = 0;
	char selfcopy[126] = { 0 };
	char existfile[126] = { 0 };
	
	GetCurrentDirectory(126, path);

	if (strcmp(argv[1],"p")==0)
		select = 1;
	else if (strcmp(argv[1], "r") == 0)
		select = 2;
	else
		select = 0;


	switch (select) {
	case 0:
		sprintf(path, "%s\\%s", path, argv[1]);
		CopyFile(path, argv[2], TRUE);
		break;
	case 1:
		sprintf(path, "%s\\%s", path, argv[2]);
		CopyFile(path, argv[3], TRUE);
		break;
	case 2:
		sprintf(existfile, "%s\\%s", path, argv[2]);
		sprintf(path, "%s\\%s*", path, argv[2]);
		handle = FindFirstFile(path, &filedata);
		sprintf(selfcopy,"%s", filedata.cFileName);
		do {	
			temp++;
		} while (FindNextFile(handle, &filedata));
		sprintf(selfcopy, "%s%d", selfcopy, temp);

		CopyFile(existfile, selfcopy, FALSE);
		break;
	}
	

	return 0;
}