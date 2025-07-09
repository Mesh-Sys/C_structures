#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

void add(void *sum, int a, int b, jmp_buf *ep){
	int c = a + b;
	printf("add - c: - %p\n", &c);
	memcpy(sum, (void*)&c, sizeof(c));
	printf("add - sum: - %p - %d\n", sum, *((int*)sum));

	longjmp(*ep, 1);
}

void *do_add(){
	void *sum = (void*)malloc(sizeof(void));
	jmp_buf *exec = (jmp_buf*)malloc(sizeof(jmp_buf));
	printf("do_add - sum: - %p\n", sum);
	printf("do_add - exec: - %p\n", exec);
	if(setjmp(*exec) == 0){
		add(sum, 10, 3, exec);
	}
	free(exec);
	return sum;
}

void *test(){
	void *value = do_add();
	printf("test - value: - %p - %d\n", value, *((int*)value));
	free(value);
}

int main(){
	void *t = test();
	printf("Returning - %p\n", t);
	return 0;
}