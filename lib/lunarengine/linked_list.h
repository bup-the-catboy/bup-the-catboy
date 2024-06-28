#ifndef LUNAR_ENGINE_LINKEDLIST_H
#define LUNAR_ENGINE_LINKEDLIST_H

#include <stdbool.h>

#define DEFINE_LIST(type) struct LinkedList_##type { \
    struct LinkedList_##type* next; \
    struct LinkedList_##type* prev; \
    struct LinkedList_##type* frst; \
    type* value; \
}

void* LE_LL_Create();
void LE_LL_Free(void* list);
void LE_LL_DeepFree(void* list, void(*dispose)(void*));
void LE_LL_Clear(void* list);
void LE_LL_DeepClear(void* list, void(*dispose)(void*));
void* LE_LL_Add(void* list, void* value);
void LE_LL_Remove(void* list, void* value);
int LE_LL_Size(void* list);
void* LE_LL_Get(void* list, int index);
void LE_LL_Sort(void* list, bool(*compare)(void*, void*));

#endif