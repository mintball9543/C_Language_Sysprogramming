#include<stdio.h>
#include<Windows.h>

int main() {
	HKEY hkey;
	DWORD keycount = 0;
	DWORD itemsize = 0;
	DWORD maxkeysize = 1024;
	DWORD maxvaluesize = 1024;
	DWORD valuecount = 0;
	DWORD valuesize = 0;

	RegOpenKeyEx(HKEY_CURRENT_USER, "console", 0, KEY_ALL_ACCESS, &hkey);
	RegQueryInfoKey(hkey, NULL, NULL, 0, &keycount, &maxkeysize, NULL, &valuecount, &itemsize, &valuesize, NULL, NULL);

	printf("console key ��: %d\n", keycount);
	printf("key�� �ִ����: %d\n", maxkeysize);
	printf("���� ��: %d\n", valuecount);
	printf("�׸��� �ִ����: %d\n", itemsize);
	printf("���� �ִ� ����: %d\n", valuesize);

	return 0;
}