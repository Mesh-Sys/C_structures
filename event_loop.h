#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ucontext.h>

#define STACK_SIZE (64*1024)

struct _queue_node_t {
	void *_data;
	struct _queue_node_t *_next;
};

struct _queue_t {
	size_t _queue_size;
	struct _queue_node_t *_first;
	struct _queue_node_t *_last;
};

struct _queue_t *_queue_create() {
	struct _queue_t *queue = (struct _queue_t*)malloc(sizeof(struct _queue_t));
	if(queue == NULL){
		printf("_queue_create - Error - Failed to create queue");
		exit(0);
	}
	queue->_queue_size = 0;
	queue->_first = queue->_last = NULL;
	return queue;
}

int _queue_empty(struct _queue_t *queue){
	return queue->_queue_size == 0;
}

void _queue_put(struct _queue_t *queue, void *data){
	struct _queue_node_t *node = (struct _queue_node_t*)malloc(sizeof(struct _queue_node_t));
	node->_data = data;
	node->_next = NULL;

	if(!queue->_first){
		queue->_first = node;
	}

	if(queue->_last){
		queue->_last->_next = node;
	}
	queue->_last = node;
	queue->_queue_size++;
}

void *_queue_get(struct _queue_t *queue){
	if(_queue_empty(queue)){return NULL;}
	struct _queue_node_t *node = queue->_first;
	void *data = node->_data;
	queue->_first = node->_next;
	free(node);
	node = NULL;
	queue->_queue_size--;
	if(!queue->_queue_size){queue->_last = NULL;}
	return data;
}

void _queue_close(struct _queue_t *queue){
	while(!_queue_empty(queue)){free(_queue_get(queue));}
	free(queue);
	queue = NULL;
}

struct coroutine_t {
	char *_stack;
	ucontext_t *_ctx;
	va_list *_args;
};

struct event_loop_t {
	struct _queue_t *_event_queue;
	ucontext_t *_main_ctx;
	char *_destroy_coroutine_stack;
	ucontext_t *_destroy_coroutine_ctx;
	struct coroutine_t *_coroutine;
	int _coroutine_finished;
};

static inline void schedule_coroutine(struct event_loop_t *ev_loop, struct coroutine_t *coroutine, void (*func)(va_list*), va_list *args);

void run_event_loop(struct event_loop_t *ev_loop){
	//printf("run_event_loop - Starting event loop - queue_size: %d\n", ev_loop->_event_queue->_queue_size);
	while(!_queue_empty(ev_loop->_event_queue)){
		ev_loop->_coroutine = _queue_get(ev_loop->_event_queue);
		//printf("run_event_loop - Running new coroutine - queue_size: %d\n", ev_loop->_event_queue->_queue_size);
		ev_loop->_coroutine_finished = 0;
		swapcontext(ev_loop->_main_ctx, ev_loop->_coroutine->_ctx);
		if(!ev_loop->_coroutine_finished){schedule_coroutine(ev_loop, (void*)ev_loop->_coroutine, NULL, NULL);}
		//printf("run_event_loop - Stopped coroutine - is_finished: %d - queue_size: %d\n", ev_loop->_coroutine_finished, ev_loop->_event_queue->_queue_size);
	}
}

void _destroy_coroutine(struct event_loop_t *ev_loop){
	while(ev_loop->_coroutine != NULL){
		if(ev_loop->_coroutine->_ctx != NULL){
			if(ev_loop->_coroutine->_args != NULL){
				va_end(*(ev_loop->_coroutine->_args));
				free(ev_loop->_coroutine->_args);
			}
			free(ev_loop->_coroutine->_stack);
			free(ev_loop->_coroutine->_ctx);
			free(ev_loop->_coroutine);
			ev_loop->_coroutine = NULL;
		}
		ev_loop->_coroutine_finished = 1;
		//printf("_destroy_coroutine - Exited coroutine - %d - queue_size: %d\n", ev_loop->_coroutine_finished, ev_loop->_event_queue->_queue_size);
		swapcontext(ev_loop->_destroy_coroutine_ctx, ev_loop->_main_ctx);
	}
}

static inline void schedule_coroutine(struct event_loop_t *ev_loop, struct coroutine_t *coroutine, void (*func)(va_list*), va_list *args){
	if(coroutine == NULL){
		coroutine = (struct coroutine_t*)malloc(sizeof(struct coroutine_t));
		coroutine->_stack = (char*)malloc(STACK_SIZE);
		coroutine->_ctx = (ucontext_t*)malloc(sizeof(ucontext_t));
		coroutine->_args = args;
		getcontext(coroutine->_ctx);
		coroutine->_ctx->uc_stack.ss_sp = coroutine->_stack;
		coroutine->_ctx->uc_stack.ss_size = STACK_SIZE;
		coroutine->_ctx->uc_link = ev_loop->_destroy_coroutine_ctx;
		makecontext(coroutine->_ctx, (void(*)(void))func, 1, coroutine->_args);
	}
	//printf("schedule_coroutine - Scheduled coroutine\n");
	_queue_put(ev_loop->_event_queue, (void*)coroutine);
}

static inline void create_coroutine(struct event_loop_t *ev_loop, void (*func)(va_list*), int argc, ...){
	//printf("await_coroutine - Awaiting coroutine - queue_size: %d\n", ev_loop->_event_queue->_queue_size);
	va_list *args = argc != 0 ? (va_list*)malloc(sizeof(va_list)) : NULL;
	if(args != NULL){va_start(*args, argc);}
	schedule_coroutine(ev_loop, NULL, func, args);
}

static inline void await_coroutine(struct event_loop_t *ev_loop, void (*func)(va_list*), int argc, ...){
	//printf("await_coroutine - Awaiting coroutine - queue_size: %d\n", ev_loop->_event_queue->_queue_size);
	va_list *args = argc != 0 ? (va_list*)malloc(sizeof(va_list)) : NULL;
	if(args != NULL){va_start(*args, argc);}
	schedule_coroutine(ev_loop, NULL, func, args);
	swapcontext(ev_loop->_coroutine->_ctx, ev_loop->_main_ctx);
}

static inline void yield_coroutine(struct event_loop_t *ev_loop){
	//printf("yield_coroutine - Yielded coroutine - queue_size: %d\n", ev_loop->_event_queue->_queue_size);
	swapcontext(ev_loop->_coroutine->_ctx, ev_loop->_main_ctx);
}

struct event_loop_t *event_loop_init(){
	struct event_loop_t *ev_loop = (struct event_loop_t*)malloc(sizeof(struct event_loop_t));
	ev_loop->_event_queue = _queue_create();
	ev_loop->_main_ctx = (ucontext_t*)malloc(sizeof(ucontext_t));
	ev_loop->_destroy_coroutine_stack = (char*)malloc(STACK_SIZE);
	ev_loop->_destroy_coroutine_ctx = (ucontext_t*)malloc(sizeof(ucontext_t));
	ev_loop->_coroutine = NULL;
	ev_loop->_coroutine_finished = 0;
	getcontext(ev_loop->_destroy_coroutine_ctx);
	ev_loop->_destroy_coroutine_ctx->uc_stack.ss_sp = ev_loop->_destroy_coroutine_stack;
	ev_loop->_destroy_coroutine_ctx->uc_stack.ss_size = STACK_SIZE;
	ev_loop->_destroy_coroutine_ctx->uc_link = ev_loop->_main_ctx;
	makecontext(ev_loop->_destroy_coroutine_ctx, (void(*)(void))_destroy_coroutine, 1, ev_loop);
	return ev_loop;
}

void event_loop_close(struct event_loop_t *ev_loop){
	_queue_close(ev_loop->_event_queue);
	free(ev_loop->_main_ctx);
	free(ev_loop->_destroy_coroutine_ctx);
	free(ev_loop->_destroy_coroutine_stack);
	free(ev_loop);
}

#endif