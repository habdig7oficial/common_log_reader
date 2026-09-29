
int hash(Hashmap *ctx, void *key){
  char *str = (char *)key;

  int acc = 0;
  for(int i = 0; str[i] != '\0'; i++){
    //printf("%c", str[i]);
    acc += (int) str[i];
  }
  return acc % ctx -> buckets;
}

bool comparable_str(void *ptr1, void *ptr2){
  return 0 == strcmp((char *)ptr1, (char *)ptr2);
}
void print_str(void *ptr){
  printf("%s ", (char *)ptr);
}

void print_long_long(void *ptr){
  printf("%lld ", *(long long *)ptr);
}
