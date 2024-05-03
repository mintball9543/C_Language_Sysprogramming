#include<stdio.h>
#include<Windows.h>
#include<time.h>
#include<tchar.h>
#define DATA_SIZE 256

typedef struct MSG_BLOCK_TAG {
	CRITICAL_SECTION mGuard;
	DWORD fReady, fStop;
	volatile DWORD nCons, mSequence;
	DWORD nLost;
	time_t mTimestamp;
	DWORD mChecksum;
	DWORD mData[DATA_SIZE];
}MSG_BLOCK;

MSG_BLOCK mBlock = { 0,0,0,0,0 };
DWORD WINAPI Produce(void*);
DWORD WINAPI Consume(void*);
void MessageFill(MSG_BLOCK*);
void MessageDisplay(MSG_BLOCK*);

DWORD _tmain(DWORD argc, LPTSTR argv[]) {
	DWORD status;
	HANDLE hProduce, hConsume;

	InitializeCriticalSection(&mBlock.mGuard);

	hProduce = (HANDLE)_beginthreadex(NULL, 0, Produce, NULL, 0, NULL);
	hConsume = (HANDLE)_beginthreadex(NULL, 0, Consume, NULL, 0, NULL);

	status = WaitForSingleObject(hConsume, INFINITE);
	status = WaitForSingleObject(hProduce, INFINITE);

	DeleteCriticalSection(&mBlock.mGuard);

	_tprintf(_T("Producer and consumer threads have terminated\n"));
	_tprintf(_T("Messages produced: %d, Consumed: %d, Lost: %d\n"), mBlock.mSequence, mBlock.nCons, mBlock.mSequence - mBlock.nCons);
	return 0;
}

DWORD WINAPI Produce(void* arg) {
	srand((DWORD)time(NULL));

	while (!mBlock.fStop) {
		Sleep(rand() / 100);

		EnterCriticalSection(&mBlock.mGuard);
		__try {
			if (!mBlock.fStop) {
				mBlock.fReady = 0;
				MessageFill(&mBlock);
				mBlock.fReady = 1;
				InterlockedIncrement(&mBlock.mSequence);
			}
		}
		__finally { LeaveCriticalSection(&mBlock.mGuard); }
	}
	return 0;
}

DWORD WINAPI Consume(void* arg) {
	CHAR command, extra;

	while (!mBlock.fStop) {
		_tprintf(_T("\n**Enter 'c' for Consume; 's' to stop: "));
		_tscanf("%c%c", &command, &extra);
		if (command == 's') {
			mBlock.fStop = 1;
		}
		else if (command == 'c') {
			EnterCriticalSection(&mBlock.mGuard);
			__try {
				if (mBlock.fReady == 0)
					_tprintf(_T("No new messages. Try again later\n"));
				else {
					MessageDisplay(&mBlock);
					mBlock.nLost = mBlock.mSequence - mBlock.nCons + 1;
					mBlock.fReady = 0;
					InterlockedIncrement(&mBlock.nCons);
				}
			}
			__finally { LeaveCriticalSection(&mBlock.mGuard); }
		}
		else {
			_tprintf(_T("Illegal command. Try again.\n"));
		}
	}
		return 0;
}

void MessageFill(MSG_BLOCK* msgBlock) {
	DWORD i;
	msgBlock->mChecksum = 0;
	for (i = 0; i < DATA_SIZE; i++) {
		msgBlock->mData[i] = rand();
		msgBlock->mChecksum ^= msgBlock->mData[i];
	}
	msgBlock->mTimestamp = time(NULL);
	return;
}

void MessageDisplay(MSG_BLOCK* msgBlock) {
	DWORD i, tcheck = 0;

	for (i = 0; i < DATA_SIZE; i++)
		tcheck ^= msgBlock->mData[i];
	_tprintf(_T("\nMessage number %d generated at: %s"), msgBlock->mSequence, _tctime(&(msgBlock->mTimestamp)));
	_tprintf(_T("First and last entries: %x %x\n"), msgBlock->mData[0], msgBlock->mData[DATA_SIZE - 1]);
	if (tcheck == msgBlock->mChecksum)
		_tprintf(_T("GOOD ->mChecksum was validataed.\n"));
	else
		_tprintf(_T("BAD ->mChecksum failed. message corrupted\n"));
	return;
}