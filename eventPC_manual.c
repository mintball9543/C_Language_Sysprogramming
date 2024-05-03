#include<stdio.h>
#include<Windows.h>
#include<time.h>
#include<tchar.h>
#define DATA_SIZE 256

typedef struct MSG_BLOCK_TAG {
	HANDLE mGuard;
	HANDLE mReady;
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
	HANDLE hProduce, hConsume;

	mBlock.mGuard = CreateMutex(NULL, FALSE, NULL);
	mBlock.mReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	hProduce = (HANDLE)_beginthreadex(NULL, 0, Produce, NULL, 0, NULL);
	hConsume = (HANDLE)_beginthreadex(NULL, 0, Consume, NULL, 0, NULL);

	WaitForSingleObject(hConsume, INFINITE);
	WaitForSingleObject(hProduce, INFINITE);

	CloseHandle(mBlock.mGuard);
	CloseHandle(mBlock.mReady);

	_tprintf(_T("Producer and consumer threads have terminated\n"));
	_tprintf(_T("Messages produced: %d, Consumed: %d, Lost: %d\n"), mBlock.mSequence, mBlock.nCons, mBlock.mSequence - mBlock.nCons);
	return 0;
}

DWORD WINAPI Produce(void* arg) {
	srand((DWORD)time(NULL));

	while (!mBlock.fStop) {
		Sleep(rand() / 5);

		ResetEvent(mBlock.mReady);
		WaitForSingleObject(mBlock.mGuard, INFINITE);
		__try {
			if (!mBlock.fStop) {
				mBlock.fReady = 0;
				MessageFill(&mBlock);
				mBlock.fReady = 1;
				InterlockedIncrement(&mBlock.mSequence);
			
				
				SetEvent(mBlock.mReady);
			}
		}
		__finally { ReleaseMutex(mBlock.mGuard); }
	}
	return 0;
}

DWORD WINAPI Consume(void* arg) {
	CHAR command[10];
	DWORD ShutDown = 0;
	while (!ShutDown) {
		_tprintf(_T("\n**Enter 'c' for Consume; 's' to stop: "));
		_tscanf(_T("%s"), &command);
		if (command[0] == 's') {
			WaitForSingleObject(mBlock.mGuard, INFINITE);
			ShutDown = mBlock.fStop = 1;
			ReleaseMutex(mBlock.mGuard);
		}
		else if (command[0] == 'c') {
			ResetEvent(mBlock.mReady);
			WaitForSingleObject(mBlock.mReady, INFINITE);
			WaitForSingleObject(mBlock.mGuard, INFINITE);
			__try {
				if (!mBlock.fReady) __leave;
				MessageDisplay(&mBlock);
				InterlockedIncrement(&mBlock.nCons);
				mBlock.nLost = mBlock.mSequence - mBlock.nCons;
				mBlock.fReady = 0;
			}
			__finally {
				ReleaseMutex(mBlock.mGuard);
				SetEvent(mBlock.mReady);
			}
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