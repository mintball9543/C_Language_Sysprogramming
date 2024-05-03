#include<stdio.h>
#include<stdlib.h>

typedef struct _data {
	int num;
	struct _data* next;
} data;

data* number;
int i = 1;

data* inputdata() {
	data* head = number;
		
	data* temp = { 0 };
	temp = (data*)malloc(sizeof(data));
	temp->next = 0;
	temp->num = 0;
	temp->num = i * 10;
	if (i == 1) {
		head = temp;
		number = head;
		
	}
	else {
		while (head->next != NULL)
			head = head->next;
		/*while (1) {
			if (head->next == NULL) break;
			head = head->next;
		}*/
		head->next = temp;
	}
	i++;

	return;
}

void printdata() {
	data* head = number;
	while (head != NULL) {
		printf("%d\n", head->num);
		head = head->next;

	}

	return;
}