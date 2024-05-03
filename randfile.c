/* Create a file of randomly keyed records.
	Each record is 64 bytes, keyed by the first 8. */
	/*  random nRec outfile	*/
	/*	MODIFIED April 12, 2010 to support huge files */

#include<stdio.h>
#include<Windows.h>
#include<wchar.h>
#include<stdarg.h>
#include<tchar.h>
#define CR 0x0D
#define LF 0x0A

static VOID srandx(DWORD);	/* MS version RAND_MAX is only */
static DWORD randx(void);	/* 32K - too small. Do our own. */

int _tmain(int argc, LPTSTR argv[])
{
	DWORD nRec, i;
	FILE* fp=0;
	DWORD iR;
	FILETIME fileTime;	/* Times to seed random #s. */
	SYSTEMTIME sysTime;
	char buffer[512] = { 0 };

	//if (argc < 3)
		//ReportError(_T("Uasge: randfile nrec file"), 1, FALSE);
	nRec = _ttoi(argv[1]);
	fp=fopen(argv[2], "wb");
		//ReportError(_T("Failed opening output."), 2, TRUE);
	GetSystemTime(&sysTime);
	SystemTimeToFileTime(&sysTime, &fileTime);

	srandx(fileTime.dwLowDateTime);
	for (i = 0; i < nRec; i++) {
		iR = randx()%4000000;
		/* 62 characters, CR, LF */
		sprintf(buffer, "%08x. Record Number: %08d.abcdefghijklmnopqrstuvwxyz x", iR, i);
		buffer[62] = CR; buffer[63] = LF;
		// _fseeki64 (fp, ((__int64)64)*i, SEEK_SET); /* Assure that an extra null character is not inserted */ /* Not needed */
		fwrite(buffer, 64, sizeof(TCHAR), fp); /* 64 characters, including CR, LF, but not EOL */
	}

	fclose(fp);
	return 0;
}

static DWORD next = 1;	/* Adapted from Plauger, p. 337. */

DWORD randx(VOID)	/* Cycle length was tested to be 2**32. */
{
	next = next * 1103515245 + 12345;
	return next;
}

VOID srandx(DWORD seed)
{
	next = seed;
	return;
}
