#include<stdio.h>
#include<Windows.h>
#include"stack.h"

int idx = 0;
int i = 0;
int j = 0;

void intputurl() {
	printf("URL을 입력하세요: ");
	while (getchar() != '\n');
	gets(stack[idx]);

	idx++;

	return;
}

void outputurl() {
	if (stack[0] == NULL) {
		printf("출력할 URL 없음\n");
		return;
	}

	printf("%s\n", stack[0]);

	for (i = 0; i < idx; i++) {
		for (j = 0;stack[i+1][j]!=NULL; j++) {
			stack[i][j] = stack[i + 1][j];
		}
	}

	if (idx == 1) memset(stack[0], 0, 256);
	idx--;

	return;
}