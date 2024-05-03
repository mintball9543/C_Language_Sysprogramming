#include<stdio.h>
#include<Windows.h>
#include<tchar.h>
#include<stdlib.h>
#include<string.h>
/* Codes for pattern meta characters. */
#define STAR        1
#define QM          2
#define BEGCLASS    3
#define ENDCLASS    4
#define ANCHOR      5
#define ASTRSK		1

/* Other codes and definitions. */
#define EOS '\0'

/* Options for pattern match. */
BOOL ignoreCase = FALSE;

#define MAX_COMMAND_LINE 256
typedef struct {
	int argc;
	TCHAR targv[4][MAX_PATH];
}GREP_THREAD_ARG;
typedef GREP_THREAD_ARG* PGR_ARGS;
static DWORD WINAPI ThGrep(PGR_ARGS pArgs);

static FILE* openFile(char* file, char* mode);
static void prepSearchString(char* p, char* buf);

int main(int argc, LPSTR argv[]) {
	GREP_THREAD_ARG* gArg;
	HANDLE* tHandle;
	DWORD threadIndex, exitCode;
	char commandLine[MAX_COMMAND_LINE];
	int iThrd, threadCount;
	STARTUPINFO startUp;
	PROCESS_INFORMATION processInfo;

	GetStartupInfo(&startUp);
	tHandle = malloc((argc - 2) * sizeof(HANDLE));
	gArg = malloc((argc - 2) * sizeof(GREP_THREAD_ARG));
	for (iThrd = 0; iThrd < argc - 2; iThrd++) {
		strcpy(gArg[iThrd].targv[0], argv[0]);
		strcpy(gArg[iThrd].targv[1], argv[1]);
		strcpy(gArg[iThrd].targv[2], argv[iThrd + 2]);
		GetTempFileName(".", "Gre", 0, gArg[iThrd].targv[3]);
		gArg[iThrd].argc = 4;

		tHandle[iThrd] = (HANDLE)_beginthreadex(NULL, 0, ThGrep, &gArg[iThrd], 0, NULL);
	}

	startUp.dwFlags = STARTF_USESTDHANDLES;
	startUp.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	threadCount = argc - 2;
	while (threadCount > 0) {
		threadIndex = WaitForMultipleObjects(threadCount, tHandle, FALSE, INFINITE);
		iThrd = (int)threadIndex - (int)WAIT_OBJECT_0;
		GetExitCodeThread(tHandle[iThrd], &exitCode);
		CloseHandle(tHandle[iThrd]);
		if (exitCode == 0) {
			if (argc > 3) {
				_tprintf(_T("\n**Search results - file: %s\n"), gArg[iThrd].targv[2]);
				fflush(stdout);
			}
			_stprintf(commandLine, _T("%s%s"), _T("C:\\Users\\USER501-08\\source\\repos\\cat\\Debug\\cat.exe "), gArg[iThrd].targv[3]);
			CreateProcess(NULL, commandLine, NULL, NULL, TRUE, 0, NULL, NULL, &startUp, &processInfo);
			WaitForSingleObject(processInfo.hProcess, INFINITE);
			CloseHandle(processInfo.hProcess);
			CloseHandle(processInfo.hThread);
		}
		DeleteFile(gArg[iThrd].targv[3]);

		tHandle[iThrd] = tHandle[threadCount - 1];
		_tcscpy(gArg[iThrd].targv[3], gArg[threadCount - 1].targv[3]);
		_tcscpy(gArg[iThrd].targv[2], gArg[threadCount - 1].targv[2]);
		threadCount--;
	}
}

static DWORD WINAPI ThGrep(PGR_ARGS pArgs) {
	/* Modified version - the first argument is the
		pattern, the second is the input file and the
		third is the output file.
		argc is not used but is assumed to be 4. */

	char* file;
	int i, patternSeen = FALSE, showName = FALSE, argc, result = 1;
	char pattern[256];
	char string[2048];
	TCHAR argv[4][MAX_COMMAND_LINE];
	FILE* fp, * fpout;

	argc = pArgs->argc;
	_tcscpy(argv[1], pArgs->targv[1]);
	_tcscpy(argv[2], pArgs->targv[2]);
	_tcscpy(argv[3], pArgs->targv[3]);
	if (argc < 3) {
		puts("Usage: grep output_file pattern file(s)");
		return 1;
	}

	/* Open the output file. */

	fpout = openFile(file = argv[argc - 1], "wb");
	if (fpout == NULL) {
		printf("Failure to open output file.");
		return 1;
	}

	for (i = 1; i < argc - 1; ++i) {
		if (argv[i][0] == _T('-')) {
			switch (argv[i][1]) {
			case _T('y'):
				ignoreCase = TRUE;
				break;
			}
		}
		else {
			if (!patternSeen++)
				prepSearchString(argv[i], pattern);
			else if ((fp = openFile(file = argv[i], _T("rb")))
				!= NULL) {
				if (!showName && i < argc - 2) ++showName;
				while (fgets(string, sizeof(string), fp)
					!= NULL && !feof(fp)) {
					if (ignoreCase) _strlwr(string);
					if (patternMatch(pattern, string)) {
						result = 0;
						if (showName) {
							fputs(file, fpout);
							fputs(string, fpout);
						}
						else fputs(string, fpout);
					}
				}
				fclose(fp);
				fclose(fpout);
			}
		}
	}
	return result;
}

static FILE* openFile(char* file, char* mode)
{
	FILE* fp;

	/* printf ("Opening File: %s", file); */

	if ((fp = fopen(file, mode)) == NULL)
		perror(file);
	return (fp);
}

static void prepSearchString(char* p, char* buf)

/* Copy prep'ed search string to buf. */
{
	register int c;
	register int i = 0;

	if (*p == _T('^')) {
		buf[i++] = ANCHOR;
		++p;
	}

	for (;;) {
		switch (c = *p++) {
		case EOS: goto Exit;
		case _T('*'): if (i >= 0 && buf[i - 1] != ASTRSK)
			c = ASTRSK; break;
		case _T('?'): c = QM; break;
		case _T('['): c = BEGCLASS; break;
		case _T(']'): c = ENDCLASS; break;

		case _T('\\'):
			switch (c = *p++) {
			case EOS: goto Exit;
			case _T('a'): c = _T('\a'); break;
			case _T('b'): c = _T('\b'); break;
			case _T('f'): c = _T('\f'); break;
			case _T('t'): c = _T('\t'); break;
			case _T('v'): c = _T('\v'); break;
			case _T('\\'): c = _T('\\'); break;
			}
			break;
		}

		buf[i++] = (ignoreCase ? tolower(c) : c);
	}

Exit:
	buf[i] = EOS;
}

static BOOL
patternMatch(char* pattern, char* string)

/* Return TRUE if pattern matches string. */
{
	register char pc, sc;
	char* pat;
	BOOL anchored;

	if (anchored = (*pattern == ANCHOR))
		++pattern;

Top:			/* Once per char in string. */
	pat = pattern;

Again:
	pc = *pat;
	sc = *string;

	if (sc == _T('\n') || sc == EOS) {
		/* at end of line or end of text */
		if (pc == EOS) goto Success;
		else if (pc == ASTRSK) {
			/* patternMatch (pat + 1,base, index, end) */
			++pat;
			goto Again;
		}
		else return (FALSE);
	}
	else {
		if (pc == sc || pc == QM) {
			/* patternMatch (pat + 1,string + 1) */
			++pat;
			++string;
			goto Again;
		}
		else if (pc == EOS) goto Success;
		else if (pc == ASTRSK) {
			if (patternMatch(pat + 1, string)) goto Success;
			else {
				/* patternMatch (pat, string + 1) */
				++string;
				goto Again;
			}
		}
		else if (pc == BEGCLASS) { /* char class */
			BOOL clmatch = FALSE;
			while (*++pat != ENDCLASS) {
				if (!clmatch && *pat == sc) clmatch = TRUE;
			}
			if (clmatch) {
				++pat;
				++string;
				goto Again;
			}
		}
	}

	if (anchored) return (FALSE);

	++string;
	goto Top;

Success:
	return (TRUE);
}