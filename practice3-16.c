#include<stdio.h>
#include<string.h>
#include<Windows.h>

int main(int argc, char* argv[]) {
	HKEY hkey[] = { HKEY_CLASSES_ROOT,HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE,HKEY_USERS,HKEY_CURRENT_CONFIG };
	char* hkeycmp[] = { "HKEY_CLASSES_ROOT","HKEY_CURRENT_USER","HKEY_LOCAL_MACHINE","HKEY_USERS","HKEY_CURRENT_CONFIG" };
	HKEY hnextkey;
	char str[512] = { 0 };
	DWORD type;
	DWORD size;
	int i = 0;
	char temp[126] = { 0 };
	DWORD data = 0;

	for (i = 0; i <= 5; i++) {
		if (i == 5) {
			printf("명령어를 다시 입력해주세요");
			return 0;
		}
		if (strcmp(argv[1], hkeycmp[i]) == 0)
			break;
	}
	RegOpenKeyEx(hkey[i], argv[2], 0, KEY_ALL_ACCESS, &hnextkey);
	RegQueryValueEx(hnextkey, argv[3], 0, &type, (BYTE*)str, &size);
	if (type == REG_DWORD){
		printf("해당하는 키 값: %x\n", str[0]);
		printf("새로 입력할 키 값을 입력하세요: ");
		scanf("%d", &data);
		

		if (RegSetValueEx(hnextkey, argv[3], 0, REG_DWORD, (char *)&data, 4) != ERROR_SUCCESS)
			printf("%d\n", GetLastError());
	}
	else {
		printf("해당하는 키 값: %s\n", str);
		printf("새로 입력할 키 값을 입력하세요: ");
		scanf("%s", temp);
		RegSetValueEx(hnextkey, argv[3], 0, type, temp, strlen(temp));
	}
	

	RegCloseKey(hkey);

	return 0;
}