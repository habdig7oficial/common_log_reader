#include "stdio.h"
#include "stdlib.h"
#include "linked_list.h"
#include "stdbool.h"

struct Hashmap;

typedef struct Pair{
  void *key;
  void *val;
} Pair;

typedef int (*hash_func)(struct Hashmap *, void *);
typedef void (*insert_func)(struct Hashmap *, void *, void *, size_t, size_t);
typedef void* (*search_func)(struct Hashmap *, void *);
typedef void (*delete_func)(struct Hashmap *, void *);
typedef bool (*comparator)(void *, void *);
typedef void* (*free_func)(struct Hashmap *);

typedef void (*printer_func)(struct Hashmap *);
typedef void (*printer_impl)(void *);

typedef void (*lambda_func)(struct Hashmap *);

typedef struct Hashmap{
  Node **hashmap;
  int buckets;
  
  hash_func hash;
  insert_func insert;
  search_func search;
  delete_func delete;
  free_func free;

  comparator comparator;

  printer_func print;
  printer_impl print_key;
  printer_impl print_val;

  lambda_func lambda;
  
  long keys;
} Hashmap;


void insert_hashmap(Hashmap *ctx, void *key, void *value, size_t size_key, size_t size_val){
  int pos = ctx -> hash(ctx, key);
  //printf("\nPos: %d\n", pos);

  //ctx -> print_key(key);
  //ctx -> print_val(value);

  void *save_key = (void *) malloc(size_key);
  void *save_val = (void *) malloc(size_val);

  memcpy(save_key, key, size_key);
  memcpy(save_val, value, size_val);

  Pair *pair = (Pair *)malloc(sizeof(Pair));
  pair -> key = save_key;
  pair -> val = save_val;

  //printf("Pair %p\n", pair);

  
  ctx -> hashmap[pos] = prepend_ptr(ctx -> hashmap[pos], pair);
  ctx -> keys++;
}

void lambda(Hashmap *ctx){
    printf("%ld", ctx -> keys);
}

void *search(Hashmap *ctx, void *key){
  Node *node = ctx -> hashmap[ctx -> hash(ctx, key)];
  while(node != NULL){
    //ctx -> print_key(((Pair *)node -> ptr_value) -> key);
    if(ctx -> comparator(((Pair *)node -> ptr_value) -> key, key))
      return ((Pair *)node -> ptr_value) -> val;
    node = node -> next;
  }
  return NULL;
}


void delete(Hashmap *ctx, void *key){
  Node *node = ctx -> hashmap[ctx -> hash(ctx, key)];
  Node *aux = NULL;
  while(node != NULL){
    //ctx -> print_key(((Pair *)node -> ptr_value) -> key);
    if(ctx -> comparator(((Pair *)node -> ptr_value) -> key, key))
      break;
    aux = node;
    node = node -> next;
  }

  if(aux == NULL)
    ctx -> hashmap[ctx -> hash(ctx, key)] = node -> next;
  else
    aux -> next = node -> next;
  
  free(((Pair *)node -> ptr_value) -> key);
  free(((Pair *)node -> ptr_value) -> val);
  free(node -> ptr_value);
  free(node);
  
  ctx -> keys--;
  return;
}

void *free_hashmap(Hashmap *ctx){
  for(int i = 0; i < ctx -> buckets; i++){
    Node *node = ctx -> hashmap[i];

    while(node != NULL){
      Node *aux = node -> next;

      free(((Pair *)node -> ptr_value) -> key);
      free(((Pair *)node -> ptr_value) -> val);
      free(node -> ptr_value);
      free(node);
      
      node = aux;
    }
  }
  free(ctx -> hashmap);
  free(ctx);
  return NULL;
}

void printer_hashmap(Hashmap *ctx){
 for(int i = 0; i < ctx -> buckets; i++){
    printf("\nBucket %d\n", i);
    if(ctx -> hashmap[i] != NULL){
      printf("\t{ ");
      ctx -> print_key(((Pair *)ctx -> hashmap[i] -> ptr_value) -> key);
      printf(": ");
      ctx -> print_val(((Pair *)ctx -> hashmap[i] -> ptr_value) -> val);
      printf("}, ");
    }
  }
}

void impl_printer_str(void *ptr){
  if(ptr != NULL)
    printf("%s", (char *)ptr);
}



Hashmap *hashmap_init(int m, hash_func h, comparator comparator, printer_impl printer_key, printer_impl printer_val){
  Hashmap *hashmap = (Hashmap *)malloc(sizeof(Hashmap));
  hashmap -> buckets = m;

  hashmap -> hashmap = (Node **)malloc(sizeof(Node *) * m);

  hashmap -> hash = h;
  hashmap -> insert = insert_hashmap;
  hashmap -> comparator = comparator;

  hashmap -> print_key = printer_key;
  hashmap -> search = search;
  hashmap -> delete = delete;
  hashmap -> free = free_hashmap;
  hashmap -> print_val = printer_val;
  hashmap -> print = printer_hashmap;

  hashmap -> lambda = lambda;
  
  for(int i = 0; i < m; i++){
    ///hashmap -> hashmap[i] = (Node *) malloc(sizeof(Node));
    hashmap -> hashmap[i] = NULL;

    //printf("%d) %p\n", i, hashmap -> hashmap[i]);
    //print_list(hashmap -> hashmap[i], int_printer);
  }


  hashmap -> keys = 0;
  
  return hashmap;
}