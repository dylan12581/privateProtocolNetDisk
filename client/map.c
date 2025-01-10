#include "head.h"
#define INIT_CAPACITY 16


HashMap* createHashMap() {
    HashMap *map = (HashMap*)malloc(sizeof(HashMap));
    map->capacity = INIT_CAPACITY;
    map->size = 0;
    map->table = (Node**)malloc(sizeof(Node*) * INIT_CAPACITY);
    memset(map->table, 0, sizeof(Node*) * INIT_CAPACITY);
    return map;
}
int hash(char* key,int capacity){
    unsigned long index = 0;
    for(int i = 0;key[i] != '\0';i++){
        index = index * 31 + key[i];
    }
    return index % capacity;
}

void insert(HashMap** table,char* key,char* value){
    int index = hash(key,(*table)->capacity);
    Node* node = (*table)->table[index];
    while(node){
        if(strcmp(node->key,key) == 0){
            strcpy(node->value,value);
            return;
        }
        node = node->next;
    }
    // 添加新的key-value
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->key = strdup(key);// 注意free
    newNode->value = strdup(value);
    // 头插法
    newNode->next = (*table)->table[index];
    (*table)->table[index] = newNode;
    (*table)->size++;
}

char* get(HashMap* map,char* key){
    int index = hash(key,map->capacity);
    Node* node = map->table[index];
    while(node){
        if(strcmp(node->key,key) == 0){
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}



