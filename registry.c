#include<stdio.h>
#include<Windows.h>
#include<stdbool.h>
#include<tchar.h>
#include<wchar.h>
#define TSIZE sizeof (TCHAR)
#define _memtchr wmemchr

BOOL TraverseRegistry(HKEY, LPTSTR, LPTSTR, LPBOOL);
BOOL DisplayPair(LPTSTR, DWORD, LPBYTE, DWORD, LPBOOL);
BOOL DisplaySubKey(LPTSTR, LPTSTR, PFILETIME, LPBOOL);
DWORD Options(int argc, LPCTSTR argv[], LPCTSTR OptStr, ...);

int _tmain(int argc, LPTSTR argv[]) {
	BOOL flags[2], ok = TRUE;
	TCHAR keyName[MAX_PATH + 1];
	LPTSTR pScan;
	DWORD i, keyIndex;
	HKEY hKey, hNextKey;

	LPTSTR PreDefKeyNames[] = {
		_T("HKEY_LOCAL_MACHINE"),_T("HEKY_CLASSES_ROOT"),_T("HKEY_CURRENT_USER"),_T("HKEY_CURRENT_CONFIG") ,NULL};

	HKEY PreDefKeys[] = { HKEY_LOCAL_MACHINE,HKEY_CLASSES_ROOT,HKEY_CURRENT_USER,HKEY_CURRENT_CONFIG };

	keyIndex = Options(argc, argv, _T("R1"), &flags[0], &flags[1], NULL);

	pScan = argv[keyIndex];
	for (i = 0; *pScan != _T('\\') && *pScan != _T('\0') && i < MAX_PATH; pScan++, i++)
		keyName[i] = *pScan;
	keyName[i] = _T('\0');
	if (*pScan == _T('\\')) pScan++;

	for (i = 0; PreDefKeyNames[i] != NULL && _tcscmp(PreDefKeyNames[i], keyName) != 0; i++);
	hKey = PreDefKeys[i];
	RegOpenKeyEx(hKey, pScan, 0, KEY_READ, &hNextKey);
	hKey = hNextKey;

	ok = TraverseRegistry(hKey, argv[keyIndex], NULL, flags);
	RegCloseKey(hKey);
	return ok ? 0 : 1;
}

BOOL TraverseRegistry(HKEY hKey, LPTSTR fullKeyName, LPTSTR subKey, LPBOOL flags) {
	HKEY hSubKey;
	BOOL recursive = flags[0];
	LONG result;
	DWORD valueType, index;
	DWORD numSubKeys, maxSubKeyLen, numValues, maxValueNameLen, maxValueLen;
	DWORD subKeyNameLen, valueNameLen, valueLen;
	FILETIME lastWriteTime;
	LPTSTR subKeyName, valueName;
	LPBYTE value;
	TCHAR fullSubKeyName[MAX_PATH + 1];

	RegOpenKeyEx(hKey, subKey, 0, KEY_READ, &hSubKey);

	RegQueryInfoKey(hSubKey, NULL, NULL, NULL, &numSubKeys, &maxSubKeyLen, NULL, &numValues, &maxValueNameLen, &maxValueLen, NULL, &lastWriteTime);
	subKeyName = malloc(TSIZE * (maxSubKeyLen + 1));
	valueName = malloc(TSIZE * (maxValueNameLen + 1));
	value = malloc(maxValueLen);

	for (index = 0; index < numValues; index++) {
		valueNameLen = maxValueNameLen + 1;
		valueLen = maxValueLen + 1;
		result = RegEnumValue(hSubKey, index, valueName, &valueNameLen, NULL, &valueType, value, &valueLen);
		if (result == ERROR_SUCCESS && GetLastError() == 0)
			DisplayPair(valueName, valueType, value, valueLen, flags);
	}
	for (index = 0; index < numSubKeys; index++) {
		subKeyNameLen = maxSubKeyLen + 1;
		result = RegEnumKeyEx(hSubKey, index, subKeyName, &subKeyNameLen, NULL, NULL, NULL, &lastWriteTime);
		if (GetLastError() == 0) {
			DisplaySubKey(fullKeyName, subKeyName, &lastWriteTime, flags);
			if (recursive) {
				_stprintf(fullSubKeyName, _T("%s\\%s"), fullKeyName, subKeyName);
				TraverseRegistry(hSubKey, fullSubKeyName, subKeyName, flags);
			}
		}
	}

	_tprintf(_T("\n"));
	free(subKeyName); free(valueName); free(value);
	RegCloseKey(hSubKey);
	return TRUE;
}

BOOL DisplayPair(LPTSTR valueName, DWORD valueType, LPBYTE value, DWORD valueLen, LPBOOL flags) {
	LPBYTE pV = value;
	DWORD i;

	_tprintf(_T("\n%s = "), valueName);

	switch (valueType) {
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_BINARY:
		for (i = 0; i < valueLen; i++, pV++)
			_tprintf(_T(" %x"), *pV);
		break;
	case REG_DWORD:
		_tprintf(_T("%x"), (DWORD)value);
		break;

	case REG_EXPAND_SZ:
	case REG_MULTI_SZ:
	case REG_SZ:
		_tprintf(_T("%s"), (LPTSTR)value);
		break;
	}
	return TRUE;
}

BOOL DisplaySubKey(LPTSTR KeyName, LPTSTR subKeyName, PFILETIME pLastWrite, LPBOOL flags) {
	BOOL longList = flags[1];
	SYSTEMTIME sysLastWrite;
	_tprintf(_T("\n%s"), KeyName);
	if (_tcslen(subKeyName) > 0) _tprintf(_T("\\%s "), subKeyName);
	if (longList) {
		FileTimeToSystemTime(pLastWrite, &sysLastWrite);
		_tprintf(_T("%02d/%02d/%04d %02d:%02d:%02d"), sysLastWrite.wMonth, sysLastWrite.wDay, sysLastWrite.wYear, sysLastWrite.wHour, sysLastWrite.wMinute, sysLastWrite.wSecond);
	}
	return TRUE;
}

DWORD Options(int argc, LPCTSTR argv[], LPCTSTR OptStr, ...)

/* argv is the command line.
	The options, if any, start with a '-' in argv[1], argv[2], ...
	OptStr is a text string containing all possible options,
	in one-to-one correspondence with the addresses of Boolean variables
	in the variable argument list (...).
	These flags are set if and only if the corresponding option
	character occurs in argv [1], argv [2], ...
	The return value is the argv index of the first argument beyond the options. */

{
	va_list pFlagList;
	LPBOOL pFlag;
	int iFlag = 0, iArg;

	va_start(pFlagList, OptStr);

	while ((pFlag = va_arg(pFlagList, LPBOOL)) != NULL
		&& iFlag < (int)_tcslen(OptStr)) {
		*pFlag = FALSE;
		for (iArg = 1; !(*pFlag) && iArg < argc && argv[iArg][0] == '-'; iArg++)
			*pFlag = _memtchr(argv[iArg], OptStr[iFlag],
				_tcslen(argv[iArg])) != NULL;
		iFlag++;
	}

	va_end(pFlagList);

	for (iArg = 1; iArg < argc && argv[iArg][0] == '-'; iArg++);

	return iArg;
}