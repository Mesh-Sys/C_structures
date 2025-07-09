#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Btree {
	int *_key;
	int *_height;
	void *_data;
	struct Btree *_left;
	struct Btree *_right;
};

struct Btree* btree_create(){
	struct Btree *root = (struct Btree*)malloc(sizeof(struct Btree));
	root->_key = root->_height = root->_data = root->_left = root->_right = NULL;
	return root;
}

struct Btree* btree_create_key(struct Btree *root, int key, int height, void *data){
	root->_key = (int*)malloc(sizeof(key));
	root->_height = (int*)malloc(sizeof(height));
	root->_data = (void*)malloc(sizeof(data));
	memcpy(root->_key, (void*)&key, sizeof(key));
	memcpy(root->_height, (void*)&height, sizeof(height));
	memcpy(root->_data, (void*)data, sizeof(data));
	return root;
}

void btree_add(struct Btree *root, int *key, void *data){
	if(root->_key == NULL){
		btree_create_key(root, *key, 1, data);
		return;
	}

	if(*key < *(root->_key)){
		if(root->_left == NULL){
			root->_left = btree_create_key(btree_create(), *key, *(root->_height) + 1, data);
		}else{
			btree_add(root->_left, key, data);
		}	
	}else{
		if(root->_right == NULL){
			root->_right = btree_create_key(btree_create(), *key, *(root->_height) + 1,  data);
		}else{
			btree_add(root->_right, key, data);
		}
	}
}

void btree_transverse(struct Btree *root){
	printf("%s Node - %d - Data: %s - Addr: %p\n\n", *(root->_height) == 1 ? "Root" : "Child", *(root->_height), (char*)root->_data, root->_left);
	if(root->_left != NULL){
		btree_transverse(root->_left);
	}

	if(root->_right != NULL){
		btree_transverse(root->_right);
	}
}

void *btree_search(struct Btree *root, int *key){
	printf("Child Node - Data: %s - Key: %d\n\n", (char*)root->_data, *(key));
	if(*key < *(root->_key)){
		return root->_left != NULL ? btree_search(root->_left, key) : NULL;
	}else if(*key > *(root->_key)){
		return root->_right != NULL ? btree_search(root->_right, key) : NULL;
	}
	return root->_data;
}

struct Btree *btree_transverse_left(struct Btree *root){
	if(root->_left != NULL){
		return btree_transverse_left(root->_left);
	}
	return root;
}

struct Btree *btree_transverse_right(struct Btree *root){
	if(root->_right != NULL){
		return btree_transverse_right(root->_right);
	}
	return root;
}

void *btree_delete(struct Btree *root, int *key){
	if(*key < *(root->_key)){
		if(root->_left != NULL){
			if(btree_delete(root->_left, key) == NULL){
				root->_left = NULL;
			}
		}
		return (void*)0xa;
	}else if(*key > *(root->_key)){
		if(root->_right != NULL){
			if(btree_delete(root->_right, key) == NULL){
				root->_right = NULL;
			}
		}
		return (void*)0xa;
	}

	free(root->_key);
	free(root->_data);
	root->_key = root->_data = NULL;
	struct Btree *_current = root->_right != NULL ? root->_right : root->_left;

	if(_current == NULL){
		if(*(root->_height) == 1){
			return NULL;
		}
		free(root);
		root = NULL;
		return NULL;
	}

	if(_current->_left != NULL){
		struct Btree *_leftmost = btree_transverse_left(_current);
		_leftmost->_left = root->_left;
	}
	root->_key = _current->_key;
	root->_data = _current->_data;
	root->_left = _current->_left;
	root->_right = _current->_right;
	free(_current->_height);
	free(_current);
}

void btree_destroy(struct Btree *root){
	if(root->_left != NULL){
		btree_destroy(root->_left);
		free(root->_left);
	}

	if(root->_right != NULL){
		btree_destroy(root->_right);
		free(root->_right);
	}
	free(root->_key);
	free(root->_height);
	free(root->_data);
}

void btree_close(struct Btree *root){
	btree_destroy(root);
	free(root);
}

int main(){
	struct Btree *root = btree_create();

	int a = 90;
	int b = 23;
	int c = 9111;
	int d = 11;
	int e = 22;
	int f = 823;
	int g = 911;
	int h = 2;
	int i = 9;
	int j = 1;

	char *ac = "v90";
	char *bc = "v23";
	char *cc = "v9111";
	char *dc = "v11";
	char *ec = "v22";
	char *fc = "v823";
	char *gc = "v911";
	char *hc = "v2";
	char *ic = "v9";
	char *jc = "v1";

	btree_add(root, &a, ac);
	btree_add(root, &b, bc);
	btree_add(root, &c, cc);
	btree_add(root, &d, dc);
	btree_add(root, &e, ec);
	btree_add(root, &f, fc);
	btree_add(root, &g, gc);
	btree_add(root, &h, hc);
	btree_add(root, &i, ic);
	btree_add(root, &j, jc);

	int pork = 01101010;

	printf("\nTransversing\n\n");
	btree_transverse(root);
	printf("\nSearching\n\n");
	void *data = btree_search(root, &j);
	printf("\nmain - Found: %s\n", data != NULL ? (char*)data : "Could not find");

	btree_delete(root, &f);

	printf("\nTransversing\n\n");
	btree_transverse(root);
	printf("\nSearching\n\n");
	data = btree_search(root, &j);
	printf("\nmain - Found: %s\n", data != NULL ? (char*)data : "Could not find");

	btree_close(root);
}