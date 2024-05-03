#include<Windows.h>
#include<stdio.h>
#include"linked.h"

int main(int argc, char* argv[]) {
	int temp;
	
	

	while (1) {
		
		scanf("%d", &temp);
		
		switch (temp) {
		case 1:
			inputdata();
			break;
		case 2:
			printdata();
			break;
		}
		if (temp == 0) break;
	}
	
	return 0;
}