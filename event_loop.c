#include <stdio.h>
#include "event_loop.h"

struct event_loop_t *event_loop;

void add(va_list *args){
	printf("add\n");
	int *a = va_arg(*args, int*);
	*a += va_arg(*args, int);
}

void sub(va_list *args){
	printf("sub\n");
	int *a = va_arg(*args, int*);
	*a -= va_arg(*args, int);
}

void do_add(va_list *args){
	//printf("do_add - First yield - queue_size: %d\n", event_loop->_event_queue->_queue_size);
	//yield_coroutine(event_loop);
	int count = 2;
	//printf("do_add - Second yield - queue_size: %d\n", event_loop->_event_queue->_queue_size);
	//yield_coroutine(event_loop);
	for(int i = 0;i < count;i++){
		int *a = (int*)malloc(sizeof(int));
		*a = -1;
		await_coroutine(event_loop, add, 2, a, i);
		printf("do_add - %d\n", *a);
		free(a);
	}
}

void do_sub(va_list *args){
	int count = 2;
	for(int i = 0;i < count;i++){
		int *a = (int*)malloc(sizeof(int)); 
		*a = -1;
		await_coroutine(event_loop, sub, 2, a, i);
		printf("do_sub - %d\n", *a);
		free(a);
	}
}

int main(){
	event_loop = event_loop_init();

	printf("main - Created event loop - %p\n", event_loop);
	create_coroutine(event_loop, do_add, 0);
	create_coroutine(event_loop, do_sub, 0);
	printf("main - Starting event loop\n");
	run_event_loop(event_loop);

	event_loop_close(event_loop);
	return 0;
}