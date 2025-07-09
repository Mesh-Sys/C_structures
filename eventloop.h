#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ucontext.h>

#define STACK_SIZE  (64*1024)

struct queue_node_t {
	void *_data;
	struct queue_node_t *_next;
};

struct queue_t {
	int _queue_size;
	struct queue_node_t *_first;
	struct queue_node_t *_last;
};

struct queue_t *queue_create() {
	struct queue_t *queue = (struct queue_t*)malloc(sizeof(struct queue_t));
	if(queue == NULL){
		printf("Failed to create queue");
		exit(0);
	}

	queue->_queue_size = 0;
	queue->_first = NULL;
	queue->_last = NULL;

	return queue;
}

int queue_qsize(struct queue_t *queue){
	return queue->_queue_size;
}

int queue_empty(struct queue_t *queue){
	return queue->_queue_size == 0;
}

void queue_put(struct queue_t *queue, void *data){
	struct queue_node_t *node = (struct queue_node_t*)malloc(sizeof(struct queue_node_t));
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

void *queue_get(struct queue_t *queue){
	if(queue_empty(queue)){
		return NULL;
	}
	struct queue_node_t *node = queue->_first;
	void *value  = node->_data;
	queue->_first = node->_next;
	free(node);
	node = NULL;
	queue->_queue_size--;
	if(!queue->_queue_size){
		queue->_last = NULL;
	}
	return value;
}

void queue_close(struct queue_t *queue){
	while(!queue_empty(queue)){
		free(queue_get(queue));
	}
	free(queue);
	queue = NULL;
}

struct coroutine_t {
	char *_stack;
	ucontext_t *_ctx;
};

struct event_loop_t {
	struct queue_t *_event_queue;
	ucontext_t *_main_ctx;
	char *_clean_stack;
	ucontext_t *_clean_ctx;
	struct coroutine_t *_task;
};

void clean_task(struct event_loop_t *ev_loop){
	while(ev_loop->_task != NULL){
		if(ev_loop->_task->_ctx != NULL){
			free(ev_loop->_task->_stack);
			free(ev_loop->_task->_ctx);
			free(ev_loop->_task);
			ev_loop->_task = NULL;
		}
		swapcontext(ev_loop->_clean_ctx, ev_loop->_main_ctx);
	}
}

void schedule_coroutine(struct event_loop_t *ev_loop, void (*func)()){
	struct coroutine_t *new_task = (struct coroutine_t*)malloc(sizeof(struct coroutine_t));
	new_task->_stack = (char*)malloc(STACK_SIZE);
	new_task->_ctx = (ucontext_t*)malloc(sizeof(ucontext_t));
	getcontext(new_task->_ctx);
	new_task->_ctx->uc_stack.ss_sp = new_task->_stack;
	new_task->_ctx->uc_stack.ss_size = STACK_SIZE;
	new_task->_ctx->uc_link = ev_loop->_clean_ctx;
	makecontext(new_task->_ctx, func, 0);
	printf("schedule_coroutine - new_task - %p\n", (void*)new_task);
	queue_put(ev_loop->_event_queue, (void*)new_task);
}

void run_event_loop(struct event_loop_t *ev_loop){
	while(!queue_empty(ev_loop->_event_queue)){
		ev_loop->_task = queue_get(ev_loop->_event_queue);
		swapcontext(ev_loop->_main_ctx, ev_loop->_task->_ctx);
		if(ev_loop->_task != NULL){queue_put(ev_loop->_event_queue, (void*)ev_loop->_task);}
	}
}

void create_coroutine(struct event_loop_t *ev_loop, void (*func)()){
	schedule_coroutine(ev_loop, func);
}

void await(struct event_loop_t *ev_loop, void (*func)()){
	schedule_coroutine(ev_loop, func);
	swapcontext(ev_loop->_task->_ctx, ev_loop->_main_ctx);
}

struct event_loop_t *event_loop_init(){
	struct event_loop_t *ev_loop = (struct event_loop_t*)malloc(sizeof(struct event_loop_t));
	ev_loop->_event_queue = queue_create();
	ev_loop->_main_ctx = (ucontext_t*)malloc(sizeof(ucontext_t));
	ev_loop->_clean_stack = (char*)malloc(STACK_SIZE);
	ev_loop->_clean_ctx = (ucontext_t*)malloc(sizeof(ucontext_t));
	ev_loop->_task = NULL;
	getcontext(ev_loop->_clean_ctx);
	ev_loop->_clean_ctx->uc_stack.ss_sp = ev_loop->_clean_stack;
	ev_loop->_clean_ctx->uc_stack.ss_size = STACK_SIZE;
	ev_loop->_clean_ctx->uc_link = ev_loop->_main_ctx;
	makecontext(ev_loop->_clean_ctx, (void(*)(void))clean_task, 1, ev_loop);
	return ev_loop;
}

void event_loop_close(struct event_loop_t *ev_loop){
	queue_close(ev_loop->_event_queue);
	free(ev_loop->_main_ctx);
	free(ev_loop->_clean_ctx);
	free(ev_loop);
}

#endif