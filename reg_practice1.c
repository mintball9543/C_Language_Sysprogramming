#include<stdio.h>
#include<Windows.h>

int main() {
	HKEY hkey;
	DWORD keystatus = REG_CREATED_NEW_KEY;

	//RegCreateKeyEx(HKEY_CURRENT_USER, "My Key", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, keystatus);

	RegOpenKeyEx(HKEY_CURRENT_USER, "My Key", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &hkey);
	RegCreateKeyEx(hkey, "test", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &keystatus);
	RegSetValueEx(hkey, "test", 0, REG_SZ, "data1", strlen("data1"));
	
	RegCloseKey(hkey);

	return 0;
}