#include <stdio.h>
#include "event_loop.h"

struct event_loop_t *event_loop;

void add(va_list *args){
	printf("add\n");
}

void sub(va_list *args){
	printf("sub\n");
}

void do_add(va_list *args){
	//printf("do_add - First yield - queue_size: %d\n", event_loop->_event_queue->_queue_size);
	//yield_coroutine(event_loop);
	int count = 2;
	//printf("do_add - Second yield - queue_size: %d\n", event_loop->_event_queue->_queue_size);
	//yield_coroutine(event_loop);
	for(int i = 0;i < count;i++){
		await_coroutine(event_loop, add, 0);
		printf("do_add - %d\n", i + i + 1);
	}
}

void do_sub(va_list *args){
	int count = 2;
	for(int i = 0;i < count;i++){
		await_coroutine(event_loop, sub, 0);
		printf("do_sub - %d\n", i - i - 1);
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