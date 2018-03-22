#include<stdio.h>
int main(void) {/*
	char name1[20];
	char name2[20];
	char name3[20];
*/
	int name1[20];
	int name2[20];
	int name3[20];
	scanf("%d", name1);
	scanf("%10d", name2);
	char c = getchar();
	//scanf("%[^\n]", name3);
	printf("name1: %d\n", name1[0]);
	printf("name2: %d\n", name2[0]);
	printf("c: %c\n", c);
	//printf("name3: %d\n", name3[0]);
}