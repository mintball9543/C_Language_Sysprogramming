#include<Windows.h>
#include<stdio.h>

int main(int argc,char* argv[]) {
	int menu = 0;

	while (1) {
		printf("1.URL�Է� 2.URL��� 3.����: ");
		scanf("%d", &menu);

		switch (menu) {
		case 1:
			intputurl();
			break;
		case 2:
			outputurl();
			break;
		case 3:
			break;
		default:
			continue;
		}

		if (menu == 3) break;
	}

	return 0;
}