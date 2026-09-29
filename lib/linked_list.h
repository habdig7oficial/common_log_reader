// Code made by Mateus Vieira for Data Struct 2 discipline
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "time.h"

typedef struct Node {
  void *ptr_value;
  struct Node *next;
} Node;


Node *new_node(void *value, int size){
  Node *node = (Node *) malloc(sizeof(Node));
  void *save = (void *) malloc(size);

  memcpy(save, value, size);
  
  node -> ptr_value = save;

  return node;
}

Node *prepend(Node *head, void *value, int size){
  Node *node = new_node(value, size);

  node -> next = head;

  return node;
}

Node *prepend_ptr(Node *head, void *value){
  Node *node = (Node *) malloc(sizeof(Node));
  node -> ptr_value = value;
  node -> next = head;

  return node;
}


typedef void (*printer)(Node *);

void print_list(Node *list, printer custom_printer){
  Node *node = list;
  while(node != NULL){
    custom_printer(node);
    node = node -> next;
  }
}

void int_printer(Node *node){
  if(node -> ptr_value != NULL)
    printf("%d\n", *(int *)(node -> ptr_value));
}

void str_printer(Node *node){
  if(node -> ptr_value != NULL)
    printf("%s\n", (char *)(node -> ptr_value));
}

void free_list(Node *list){
  Node *node = list;
  while(node != NULL){
    free(node -> ptr_value);
    Node *old = node;
    node = node -> next;

    free(old);
  }
}