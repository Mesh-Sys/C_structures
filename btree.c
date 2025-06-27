#include <stdio.h>
#include <string.h>

struct Node {
	int *_key;
	struct Node *_left;
	struct Node *_right;
};

struct Node* btree_create(){
	struct Node *root = (struct Node*)malloc(sizeof(struct Node));
	root->_left = NULL;
	root->_right = NULL;
	return root;
}

void btree_add(struct Node *root, int *data){

}

int main(){
	struct Node *root = btree_create();
	free(root);
}