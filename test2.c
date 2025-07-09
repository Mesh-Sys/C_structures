#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include <string.h>

struct Node {
	void *_data;
	struct Node *_next;
};

struct Queue {
	int _queue_size;
	struct Node *_first;
	struct Node *_last;
};

struct Queue *queue_create() {
	struct Queue *queue = (struct Queue*)malloc(sizeof(struct Queue));
	if(queue == NULL){
		printf("Failed to create queue");
		exit(0);
	}

	queue->_queue_size = 0;
	queue->_first = NULL;
	queue->_last = NULL;

	return queue;
}

int queue_qsize(struct Queue *queue){
	return queue->_queue_size;
}

int queue_empty(struct Queue *queue){
	return queue->_queue_size == 0;
}

void queue_put(struct Queue *queue, void *data){
	struct Node *node = (struct Node*)malloc(sizeof(struct Node));
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

void *queue_get(struct Queue *queue){
	if(queue_empty(queue)){
		return NULL;
	}
	struct Node *node = queue->_first;
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

void queue_close(struct Queue *queue){
	free(queue);
	queue = NULL;
}

struct Queue *event_queue;

struct Task {
	void (*_func)(va_list*);
	va_list *_args;
	jmp_buf *_exec_point;
};

void run_event_loop(){
	while(!queue_empty(event_queue)){
		struct Task *task = queue_get(event_queue);
		(task->_func)(task->_args);
		va_end(*(task->_args));
		free(task->_args);
		jmp_buf *exec_point = NULL;
		if(task->_exec_point != NULL){
			exec_point = task->_exec_point;
		}
		free(task);
		if(exec_point != NULL){
			printf("run_event_loop - Exec point - %p\n", exec_point);
			//longjmp(*exec_point, 1);
		}
	}
}

void add_task_to_queue(void (*func)(va_list*), va_list *args, jmp_buf *exec_point){
	struct Task *new_task = (struct Task*)malloc(sizeof(struct Task));
	new_task->_func = func;
	new_task->_args = args;
	new_task->_exec_point = exec_point;
	printf("add_task_to_queue - new_task - %p\n", (void*)new_task);
	queue_put(event_queue, (void*)new_task);
}

void create_task(void (*func)(va_list*), ...){
	va_list *args = (va_list*)malloc(sizeof(va_list));
	va_start(*args, func);
	add_task_to_queue(func, args, NULL);
}

void await(void (*func)(va_list*), ...){
	va_list *args = (va_list*)malloc(sizeof(va_list));
	va_start(*args, func);
	jmp_buf *exec_point = (jmp_buf*)malloc(sizeof(jmp_buf));
	if(!setjmp(*exec_point)){
		add_task_to_queue(func, args, exec_point);
		run_event_loop();
	}
	free(exec_point);
}

void add(va_list *args){
	printf("add - value: - %d\n", *(va_arg(*args, int*)) + va_arg(*args, int));
}

void sub(va_list *args){
	printf("sub - value: - %d\n", *(va_arg(*args, int*)) - va_arg(*args, int));
}

void do_add(va_list *args){
	int count = 10;
	for(int i = 0;i < count;i++){
		await(add, &i, i + 1);
		printf("do_add - %d\n", i + i + 1);
	}
}

void do_sub(va_list *args){
	int count = 10;
	for(int i = 0;i < count;i++){
		await(sub, &i, i - 1);
		printf("do_sub - %d\n", i - i - 1);
	}
}

int main(){
	event_queue = queue_create();
	printf("main - Created event loop - %p\n", event_queue);
	create_task(do_add);
	create_task(do_sub);
	printf("main - Starting event loop\n");
	run_event_loop();
	queue_close(event_queue);
	return 0;
}