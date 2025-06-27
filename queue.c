#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct Node {
	void *_data;
	struct Node *_next;
};

struct Queue {
	int _max_size;
	int _queue_size;
	struct Node *_first;
	struct Node *_last;

	pthread_mutex_t _queue_lock;
	pthread_cond_t _queue_cond;
};

struct Queue *queue_create(int *max_size) {
	struct Queue *queue = (struct Queue*)malloc(sizeof(struct Queue));
	if(queue == NULL){
		printf("Failed to create queue");
		exit(0);
	}

	queue->_max_size = max_size == NULL ? 0 : *max_size;
	queue->_queue_size = 0;
	queue->_first = NULL;
	queue->_last = NULL;

	if(pthread_mutex_init(&(queue->_queue_lock), NULL) != 0){
		printf("Failed to initialize queue mutex\n");
		exit(0);
	}

	if(pthread_cond_init(&(queue->_queue_cond), NULL) != 0){
		printf("Failed to initialize queue condition\n");
		exit(0);
	}

	return queue;
}

int queue_qsize(struct Queue *queue){
	return queue->_queue_size;
}

int queue_empty(struct Queue *queue){
	return queue->_queue_size == 0;
}

int queue_full(struct Queue *queue){
	if(queue->_max_size != 0){
		return queue->_queue_size == queue->_max_size;
	}
	return 0;
}

void queue_put(struct Queue *queue, void *data){
	pthread_mutex_lock(&(queue->_queue_lock));
	if(queue->_max_size != 0){
		while(queue_qsize(queue) == queue->_max_size){
			pthread_cond_wait(&(queue->_queue_cond), &(queue->_queue_lock));
		}
	}

	struct Node *node = (struct Node*)malloc(sizeof(struct Node));
	node->_data = (void*)malloc(sizeof(data));
	memcpy(node->_data, data, sizeof(data));
	node->_next = NULL;

	if(!queue->_first){
		queue->_first = node;
	}

	if(queue->_last){
		queue->_last->_next = node;
	}
	queue->_last = node;
	queue->_queue_size++;

	if(!queue->_queue_size){
		pthread_cond_signal(&(queue->_queue_cond));
	}
	pthread_mutex_unlock(&(queue->_queue_lock));
}

void *queue_get(struct Queue *queue){
	pthread_mutex_lock(&(queue->_queue_lock));
	if(!queue->_queue_size){
		pthread_cond_wait(&(queue->_queue_cond), &(queue->_queue_lock));
	}

	if(queue->_max_size != 0){
		if(queue_qsize(queue) == queue->_max_size){
			pthread_cond_signal(&(queue->_queue_cond));
		}
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
	pthread_mutex_unlock(&(queue->_queue_lock));
	return value;
}

void queue_close(struct Queue *queue){
	pthread_mutex_destroy(&(queue->_queue_lock));
	pthread_cond_destroy(&(queue->_queue_cond));
	free(queue);
	queue = NULL;
}

int main() {
	char str[] = "jijijeiejzeiiiji";
	struct Queue *queue = queue_create(NULL);
	printf("Created queue\n");
	int i = 8;
	printf("Current address - %p\n", &i);
	queue_put(queue, &str);
	i = 99;
	printf("Current data - %s\n", (char*)queue_get(queue));
	queue_close(queue);
	return 0;
}