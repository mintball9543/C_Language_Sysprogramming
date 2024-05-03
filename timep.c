#include<stdio.h>
#include<Windows.h>
#include<tchar.h>

LPTSTR SkipArg(LPTSTR cLine, int argn, int argc, LPTSTR argv[]);

int _tmain(int argc, LPTSTR argv[]) {
	STARTUPINFO startUp;
	PROCESS_INFORMATION procInfo;
	union {
		LONGLONG li;
		FILETIME ft;
	}createTime, exitTime, elapsedTime;

	FILETIME kernelTime, userTime;
	SYSTEMTIME elTiSys, keTiSys, usTiSys, startTimeSys;
	LPTSTR targv = SkipArg(GetCommandLine(), 1, argc, argv);
	HANDLE hProc;

	GetStartupInfo(&startUp);
	GetSystemTime(&startTimeSys);

	CreateProcess(NULL, targv, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startUp, &procInfo);
	hProc = procInfo.hProcess;
	WaitForSingleObject(hProc, INFINITE);

	GetProcessTimes(hProc, &createTime.ft, &exitTime.ft, &kernelTime, &userTime);
	elapsedTime.li = exitTime.li - createTime.li;
	FileTimeToSystemTime(&elapsedTime.ft, &elTiSys);
	FileTimeToSystemTime(&kernelTime, &keTiSys);
	FileTimeToSystemTime(&userTime, &usTiSys);
	printf("\n");
	_tprintf(_T("Real Time: %02d:%02d:%02d:%03d\n"), elTiSys.wHour, elTiSys.wMinute, elTiSys.wSecond, elTiSys.wMilliseconds);
	_tprintf(_T("User Time: %02d:%02d:%02d:%03d\n"), usTiSys.wHour, usTiSys.wMinute, usTiSys.wSecond, usTiSys.wMilliseconds);
	_tprintf(_T("Sys Time: %02d:%02d:%02d:%03d\n"), keTiSys.wHour, keTiSys.wMinute, keTiSys.wSecond, keTiSys.wMilliseconds);

	CloseHandle(procInfo.hThread);
	CloseHandle(procInfo.hProcess);
	CloseHandle(hProc);

	return 0;
}

LPTSTR SkipArg(LPTSTR cLine, int argn, int argc, LPTSTR argv[])
{
	LPTSTR pArg = cLine, cEnd = pArg + _tcslen(cLine);
	int iArg;

	if (argn >= argc) return NULL;

	for (iArg = 0; iArg < argn && pArg < cEnd; iArg++)
	{
		if ('"' == *pArg)
		{
			pArg += _tcslen(argv[iArg]) + 2; /* Skip over the argument and the enclosing quotes */
		}
		else
		{
			pArg += _tcslen(argv[iArg]); /* Skip over the argument */
		}

		while ((pArg < cEnd) && ((_T(' ') == *pArg) || (_T('\t')) == *pArg)) pArg++;
	}

	if (pArg >= cEnd) return NULL;
	return pArg;
}