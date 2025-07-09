#include <stdio.h>
#include "eventloop.h"

struct event_loop_t *event_loop;

void add(){
	printf("add\n");
}

void sub(){
	printf("sub\n");
}

void do_add(){
	int count = 2;
	for(int i = 0;i < count;i++){
		await(event_loop, add);
		printf("do_add - %d\n", i + i + 1);
	}
}

void do_sub(){
	int count = 2;
	for(int i = 0;i < count;i++){
		await(event_loop, sub);
		printf("do_sub - %d\n", i - i - 1);
	}
}

int main(){
	event_loop = event_loop_init();

	printf("main - Created event loop - %p\n", event_loop);
	create_coroutine(event_loop, do_add);
	create_coroutine(event_loop, do_sub);
	printf("main - Starting event loop\n");
	run_event_loop(event_loop);

	event_loop_close(event_loop);
	return 0;
}