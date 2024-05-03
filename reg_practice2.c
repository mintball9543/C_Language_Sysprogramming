#include<stdio.h>
#include<Windows.h>

int main() {
	HKEY hkey;
	char name[] = "C:\\windows\\notepad.exe";
	DWORD type = REG_SZ;
	DWORD size = 1024;
	char buffer[1024] = {0};
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, "software\\microsoft\\windows\\currentversion\\run", 0, KEY_ALL_ACCESS, &hkey);
	//RegCreateKeyEx(hkey, "my program", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkey, REG_CREATED_NEW_KEY);
	if (RegQueryValueEx(hkey, "my program", 0, &type, (BYTE*)buffer, &size) != ERROR_SUCCESS)
		RegSetValueEx(hkey, "my program", 0, REG_SZ, name, strlen(name));
	else {
		system("C:\\windows\\system32\\calc.exe");
		exit(0);
	}
	RegCloseKey(hkey);

	return 0;
}