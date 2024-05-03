#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include<stdarg.h>
#include"support.h"
#include"JobManagement.h"

#define MILLION 1000000

DWORD Options(int argc, LPCTSTR argv[], LPCTSTR OptStr, ...);
LPTSTR SkipArg(LPTSTR cLine, int argn, int argc, LPTSTR argv[]);

HANDLE hJobObject = NULL;

JOBOBJECT_BASIC_LIMIT_INFORMATION basicLimits = { 0,0,JOB_OBJECT_LIMIT_PROCESS_TIME };

int _tmain(int argc,LPTSTR argv[]) {
	LARGE_INTEGER processTimeLimit;
	BOOL exitFlag = FALSE;
	TCHAR command[MAX_COMMAND_LINE], * pc;
	DWORD i, localArgc;
	TCHAR argstr[MAX_ARG][MAX_COMMAND_LINE];
	LPTSTR pArgs[MAX_ARG];


	hJobObject = NULL;
	processTimeLimit.QuadPart = 0;
	if (argc >= 2)processTimeLimit.QuadPart = atoi(argv[1]);
	basicLimits.PerProcessUserTimeLimit.QuadPart = processTimeLimit.QuadPart * 10 * MILLION;

	hJobObject = CreateJobObject(NULL, NULL);
	SetInformationJobObject(hJobObject, JobObjectBasicLimitInformation, &basicLimits, sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION));

	for (i = 0; i < MAX_ARG; i++) pArgs[i] = argstr[i];
	_tprintf(_T("Windows Job Management\n"));
	while (!exitFlag) {
		_tprintf(_T("%s"), _T("JM$"));
		_fgetts(command, MAX_COMMAND_LINE, stdin);
		pc = wcschr(command, '\n');
		*pc = '\0';
		GetArgs(command, &localArgc, pArgs);
		CharLower(argstr[0]);
		if (_tcscmp(argstr[0], _T("jobbg")) == 0)
			Jobbg(localArgc, pArgs, command);
		else if (_tcscmp(argstr[0], _T("jobs")) == 0)
			Jobs(localArgc, pArgs, command);
		else if (_tcscmp(argstr[0], _T("kill")) == 0)
			Kill(localArgc, pArgs, command);
		else if (_tcscmp(argstr[0], _T("quit")) == 0)
			exitFlag = TRUE;
		else _tprintf(_T("Illegal command. Try again\n"));
	}


	CloseHandle(hJobObject);

	return 0;
}

int Kill(int argc, LPTSTR argv[], LPTSTR command) {
	DWORD ProcessId, jobNumber, iJobNo;
	HANDLE hProcess;
	BOOL cntrlC, cntrlB;

	iJobNo = Options(argc, argv, _T("bc"), &cntrlB, &cntrlC, NULL);

	jobNumber = _ttoi(argv[iJobNo]);
	ProcessId = FindProcessId(jobNumber);
	hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, ProcessId);
	if (hProcess == NULL) {
		ReportError(_T("Process already terminated.\n"), 0, FALSE);
		return 2;
	}
	if (cntrlB)
		GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, ProcessId);
	else if (cntrlC)
		GenerateConsoleCtrlEvent(CTRL_C_EVENT, ProcessId);
	TerminateProcess(hProcess, JM_EXIT_CODE);
	WaitForSingleObject(hProcess, 5000);
	CloseHandle(hProcess);
	_tprintf(_T("Job [%d] terminated or timed out \n"), jobNumber);
	return 0;

}

int Jobbg(int argc, LPTSTR argv[], LPTSTR command) {
	DWORD fCreate;
	LONG jobNumber;
	BOOL flags[2];

	STARTUPINFO startUp;
	PROCESS_INFORMATION processInfo;
	LPTSTR targv = SkipArg(command,1,argc,argv);

	GetStartupInfo(&startUp);

	Options(argc, argv, _T("cd"), &flags[0], &flags[1], NULL);

	if (argv[1][0] == '-')
		targv = SkipArg(targv,1,argc,argv);

	fCreate = flags[0] ? CREATE_NEW_CONSOLE : flags[1] ? DETACHED_PROCESS : 0;

	CreateProcess(NULL, targv, NULL, NULL, TRUE, fCreate | CREATE_SUSPENDED | CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startUp, &processInfo);

	AssignProcessToJobObject(hJobObject, processInfo.hProcess);

	jobNumber = GetJobNumber(&processInfo, targv);
	if (jobNumber >= 0)
		ResumeThread(processInfo.hThread);
	else {
		TerminateProcess(processInfo.hProcess, 3);
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
		return 5;
	}
	CloseHandle(processInfo.hThread);
	CloseHandle(processInfo.hProcess);
	_tprintf(_T("[%d] %d\n"), jobNumber, processInfo.dwProcessId);
	return 0;
}

int Jobs(int argc, LPTSTR argv[], LPTSTR command) {
	JOBOBJECT_BASIC_ACCOUNTING_INFORMATION BasicInfo;

	DisplayJobs();

	QueryInformationJobObject(hJobObject, JobObjectBasicAccountingInformation, &BasicInfo, sizeof(JOBOBJECT_BASIC_ACCOUNTING_INFORMATION), NULL);
	_tprintf(_T("Total Processes: %d, Active: %d, Terminated: %d.\n"), BasicInfo.TotalProcesses, BasicInfo.ActiveProcesses, BasicInfo.TotalTerminatedProcesses);
	_tprintf(_T("User time all processes: %d.%03d\n"), BasicInfo.TotalUserTime.QuadPart / MILLION, (BasicInfo.TotalUserTime.QuadPart % MILLION) / 10000);

	return 0;
}


BOOL GetJobMgtFileName(LPTSTR jobMgtFileName)

/* Create the name of the job management file in the temporary directory. */
{
	TCHAR UserName[MAX_PATH], TempPath[MAX_PATH];
	DWORD UNSize = MAX_PATH, TPSize = MAX_PATH;

	if (!GetUserName(UserName, &UNSize))
		return FALSE;
	if (GetTempPath(TPSize, TempPath) > TPSize)
		return FALSE;
	_stprintf_s(jobMgtFileName, MAX_PATH, _T("%s%s%s"), TempPath, UserName, _T(".JobMgt"));
	return TRUE;
}


LONG GetJobNumber(PROCESS_INFORMATION* pProcessInfo, LPCTSTR command) {
	HANDLE hJobData, hProcess;
	JM_JOB jobRecord;
	DWORD jobNumber = 0, nXfer, exitCode, fileSizeLow, fileSizeHigh;
	TCHAR jobMgtFileName[MAX_PATH];
	OVERLAPPED regionStart;

	if (!GetJobMgtFileName(jobMgtFileName)) return -1;
	hJobData = CreateFile(jobMgtFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hJobData == INVALID_HANDLE_VALUE) return -1;

	regionStart.Offset = 0;
	regionStart.OffsetHigh = 0;
	regionStart.hEvent = (HANDLE)0;

	fileSizeLow = GetFileSize(hJobData, &fileSizeHigh);
	LockFileEx(hJobData, LOCKFILE_EXCLUSIVE_LOCK, 0, fileSizeLow + SJM_JOB, 0, &regionStart);

	__try {
		while (ReadFile(hJobData, &jobRecord, SJM_JOB, &nXfer, NULL) && (nXfer > 0)) {
			printf("%d\n", GetLastError());
			if (jobRecord.ProcessId == 0) break;
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, jobRecord.ProcessId);
			if (hProcess == NULL) break;
			if (GetExitCodeProcess(hProcess, &exitCode) && (exitCode != STILL_ACTIVE)) 
				break;
			jobNumber++;
		}

		if (nXfer != 0)
			SetFilePointer(hJobData, -(LONG)SJM_JOB, NULL, FILE_CURRENT);
		jobRecord.ProcessId = pProcessInfo->dwProcessId;
		_tcsnccpy(jobRecord.CommandLine, command, MAX_PATH);
		WriteFile(hJobData, &jobRecord, SJM_JOB, &nXfer, NULL);
	}
	__finally {
		UnlockFileEx(hJobData, 0, fileSizeLow + SJM_JOB, 0, &regionStart);
		CloseHandle(hJobData);
	}
	return jobNumber + 1;
}

BOOL DisplayJobs(void) {
	HANDLE hJobData, hProcess;
	JM_JOB jobRecord;
	DWORD jobNumber = 0, nXfer, exitCode, fileSizeLow, fileSizeHigh;
	TCHAR jobMgtFileName[MAX_PATH];
	OVERLAPPED regionStart;

	GetJobMgtFileName(jobMgtFileName);
	hJobData = CreateFile(jobMgtFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	regionStart.Offset = 0;
	regionStart.OffsetHigh = 0;
	regionStart.hEvent = (HANDLE)0;

	fileSizeLow = GetFileSize(hJobData, &fileSizeHigh);
	LockFileEx(hJobData, LOCKFILE_EXCLUSIVE_LOCK, 0, fileSizeLow, fileSizeHigh, &regionStart);

	__try {
		while (ReadFile(hJobData, &jobRecord, SJM_JOB, &nXfer, NULL) && (nXfer > 0)) {
			jobNumber++;
			if (jobRecord.ProcessId == 0)
				continue;
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, jobRecord.ProcessId);
			if (hProcess != NULL)
				GetExitCodeProcess(hProcess, &exitCode);
			_tprintf(_T(" [%d] "), jobNumber);
			if (hProcess == NULL)
				_tprintf(_T(" Done"));
			else if (exitCode != STILL_ACTIVE)
				_tprintf(_T("+ Done"));
			else _tprintf(_T(" "));
			_tprintf(_T("%s\n"), jobRecord.CommandLine);
			if (hProcess == NULL) {
				SetFilePointer(hJobData, -(LONG)nXfer, NULL, FILE_CURRENT);
				jobRecord.ProcessId = 0;
				WriteFile(hJobData, &jobRecord, SJM_JOB, &nXfer, NULL);
			}
		}
	}

	__finally {
		UnlockFileEx(hJobData, 0, fileSizeLow, fileSizeHigh, &regionStart);
		CloseHandle(hJobData);
	}
	return TRUE;
}

DWORD FindProcessId(DWORD jobNumber) {
	HANDLE hJobData;
	JM_JOB jobRecord;
	DWORD nXfer;
	TCHAR jobMgtFileName[MAX_PATH];
	OVERLAPPED regionStart;

	GetJobMgtFileName(jobMgtFileName);

	hJobData = CreateFile(jobMgtFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hJobData == INVALID_HANDLE_VALUE) return 0;
	SetFilePointer(hJobData, SJM_JOB * (jobNumber - 1), NULL, FILE_BEGIN);

	regionStart.Offset = SJM_JOB * (jobNumber - 1);
	regionStart.OffsetHigh = 0;
	regionStart.hEvent = (HANDLE)0;
	LockFileEx(hJobData, 0, 0, SJM_JOB, 0, &regionStart);
	ReadFile(hJobData, &jobRecord, SJM_JOB, &nXfer, NULL);
	UnlockFile(hJobData, 0, SJM_JOB, 0, &regionStart);
	CloseHandle(hJobData);
	return jobRecord.ProcessId;
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
		for (iArg = 1; !(*pFlag) && iArg < argc && argv[iArg][0] == _T('-'); iArg++)
			*pFlag = _memtchr(argv[iArg], OptStr[iFlag],
				_tcslen(argv[iArg])) != NULL;
		iFlag++;
	}

	va_end(pFlagList);

	for (iArg = 1; iArg < argc && argv[iArg][0] == _T('-'); iArg++);

	return iArg;
}



VOID ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printErrorMessage)

/* General-purpose function for reporting system errors.
	Obtain the error number and convert it to the system error message.
	Display this information and the user-specified message to the standard error device.
	userMessage:		Message to be displayed to standard error device.
	exitCode:		0 - Return.
					> 0 - ExitProcess with this code.
	printErrorMessage:	Display the last system error message if this flag is set. */
{
	DWORD eMsgLen, errNum = GetLastError();
	LPTSTR lpvSysMsg;
	_ftprintf(stderr, _T("%s\n"), userMessage);
	if (printErrorMessage) {
		eMsgLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, errNum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpvSysMsg, 0, NULL);
		if (eMsgLen > 0)
		{
			_ftprintf(stderr, _T("%s\n"), lpvSysMsg);
		}
		else
		{
			_ftprintf(stderr, _T("Last Error Number; %d.\n"), errNum);
		}

		if (lpvSysMsg != NULL) LocalFree(lpvSysMsg); /* Explained in Chapter 5. */
	}

	if (exitCode > 0)
		ExitProcess(exitCode);

	return;
}

/* Extension of ReportError to generate a user exception
	code rather than terminating the process. */

VOID ReportException(LPCTSTR userMessage, DWORD exceptionCode)

/* Report as a non-fatal error.
	Print the system error message only if the message is non-null. */
{
	if (lstrlen(userMessage) > 0)
		ReportError(userMessage, 0, TRUE);
	/* If fatal, raise an exception. */

	if (exceptionCode != 0)
		RaiseException(
			(0x0FFFFFFF & exceptionCode) | 0xE0000000, 0, 0, NULL);

	return;
}

BOOL PrintStrings(HANDLE hOut, ...)
/* Write the messages to the output handle. Frequently hOut
	will be standard out or error, but this is not required.
	Use WriteConsole (to handle Unicode) first, as the
	output will normally be the console. If that fails, use WriteFile.

	hOut:	Handle for output file.
	... :	Variable argument list containing TCHAR strings.
		The list must be terminated with NULL. */
{
	DWORD msgLen, count;
	LPCTSTR pMsg;
	va_list pMsgList;	/* Current message string. */
	va_start(pMsgList, hOut);	/* Start processing msgs. */
	while ((pMsg = va_arg(pMsgList, LPCTSTR)) != NULL) {
		msgLen = lstrlen(pMsg);
		if (!WriteConsole(hOut, pMsg, msgLen, &count, NULL)
			&& !WriteFile(hOut, pMsg, msgLen * sizeof(TCHAR), &count, NULL)) {
			va_end(pMsgList);
			return FALSE;
		}
	}
	va_end(pMsgList);
	return TRUE;
}


BOOL PrintMsg(HANDLE hOut, LPCTSTR pMsg)

/* For convenience only - Single message version of PrintStrings so that
	you do not have to remember the NULL arg list terminator.

	hOut:	Handle of output file
	pMsg:	Single message to output. */
{
	return PrintStrings(hOut, pMsg, NULL);
}


BOOL ConsolePrompt(LPCTSTR pPromptMsg, LPTSTR pResponse, DWORD maxChar, BOOL echo)

/* Prompt the user at the console and get a response
	which can be up to maxChar generic characters.

	pPromptMessage:	Message displayed to user.
	pResponse:	Programmer-supplied buffer that receives the response.
	maxChar:	Maximum size of the user buffer, measured as generic characters.
	echo:		Do not display the user's response if this flag is FALSE. */
{
	HANDLE hIn, hOut;
	DWORD charIn, echoFlag;
	BOOL success;
	hIn = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	hOut = CreateFile(_T("CONOUT$"), GENERIC_WRITE, 0,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	/* Should the input be echoed? */
	echoFlag = echo ? ENABLE_ECHO_INPUT : 0;

	/* API "and" chain. If any test or system call fails, the
		rest of the expression is not evaluated, and the
		subsequent functions are not called. GetStdError ()
		will return the result of the failed call. */

	success = SetConsoleMode(hIn, ENABLE_LINE_INPUT | echoFlag | ENABLE_PROCESSED_INPUT)
		&& SetConsoleMode(hOut, ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_PROCESSED_OUTPUT)
		&& PrintStrings(hOut, pPromptMsg, NULL)
		&& ReadConsole(hIn, pResponse, maxChar - 2, &charIn, NULL);

	/* Replace the CR-LF by the null character. */
	if (success)
		pResponse[charIn - 2] = _T('\0');
	else
		ReportError(_T("ConsolePrompt failure."), 0, TRUE);

	CloseHandle(hIn);
	CloseHandle(hOut);
	return success;
}


void GetArgs(LPCTSTR Command, int* pArgc, LPTSTR argstr[])
{
	int i, icm = 0;
	DWORD ic = 0;

	for (i = 0; ic < _tcslen(Command); i++) {
		while (ic < _tcslen(Command) &&
			Command[ic] != _T(' ') && Command[ic] != _T('\t')) {
			argstr[i][icm] = Command[ic];
			ic++;
			icm++;
		}
		argstr[i][icm] = _T('\0');
		while (ic < _tcslen(Command) &&
			(Command[ic] == _T(' ') || Command[ic] == _T('\t')))
			ic++;
		icm = 0;
	}

	if (pArgc != NULL) *pArgc = i;
	return;
}