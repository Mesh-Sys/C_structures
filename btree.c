#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
	int *_key;
	struct Node *_left;
	struct Node *_right;
};

struct Node* btree_create(){
	struct Node *root = (struct Node*)malloc(sizeof(struct Node));
	root->_key = NULL;
	root->_left = NULL;
	root->_right = NULL;
	return root;
}

struct Node* btree_create_key(struct Node *root, int *data){
	root->_key = (int*)malloc(sizeof(data));
	memcpy(root->_key, data, sizeof(data));	
	return root;
}

void btree_add(struct Node *root, int *data){
	if(root->_key == NULL){
		btree_create_key(root, data);
		printf("Root - Set new key - src: %p - dest: %p - value: %d\n", data, root->_key, *(root->_key));
		return;
	}

	if(*data < *(root->_key)){
		if(root->_left == NULL){
			root->_left = btree_create_key(btree_create(), data);
			printf("Child - Left - Set new key - src: %p - dest: %p - value: %d\n", data, root->_left->_key, *(root->_left->_key));
		}else{
			btree_add(root->_left, data);
		}
	}else{
		if(root->_right == NULL){
			root->_right = btree_create_key(btree_create(), data);
			printf("Child - Right - Set new key - src: %p - dest: %p - value: %d\n", data, root->_right->_key, *(root->_right->_key));
		}else{
			btree_add(root->_right, data);
		}
	}
}

void btree_transverse(struct Node *root, int height){
	printf("%s Node - %d - Data: %d\n\n", height == 1 ? "Root" : "Child", height, *(root->_key));
	if(root->_left != NULL){
		btree_transverse(root->_left, height + 1);
	}

	if(root->_right != NULL){
		btree_transverse(root->_right, height + 1);
	}
}

int main(){
	struct Node *root = btree_create();
	int a = 90;
	int b = 23;
	int c = 9111;
	int d = 1;
	int e = 2;
	int f = 823;
	int g = 911;
	btree_add(root, &a);
	btree_add(root, &b);
	btree_add(root, &c);
	btree_add(root, &d);
	btree_add(root, &e);
	btree_add(root, &f);
	btree_add(root, &g);
	printf("\nTransversing\n\n");
	btree_transverse(root, 1);
	free(root);
}