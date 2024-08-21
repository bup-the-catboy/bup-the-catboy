#include <stdlib.h>

#include "linked_list.h"

DEFINE_LIST(void);

void* LE_LL_Create() {
    struct LinkedList_void* list = malloc(sizeof(struct LinkedList_void));
    list->next = NULL;
    list->prev = NULL;
    list->value = NULL;
    list->frst = list;
    return list;
}

void _Dispose(void* x) {}

void LE_LL_Free(void* list) {
    LE_LL_DeepFree(list, _Dispose);
}

void LE_LL_DeepFree(void* list, void(*dispose)(void*)) {
    struct LinkedList_void* ll = list;
    while (ll) {
        struct LinkedList_void* next = ll->next;
        if (ll->value) dispose(ll->value);
        free(ll);
        ll = next;
    }
}

void LE_LL_Clear(void* list) {
    LE_LL_DeepClear(list, _Dispose);
}

void LE_LL_DeepClear(void* list, void(*dispose)(void*)) {
    struct LinkedList_void* ll = list;
    while (ll) {
        struct LinkedList_void* next = ll->next;
        if (ll->prev) {
            if (ll->value) dispose(ll->value);
            free(ll);
        }
        else ll->next = NULL;
        ll = next;
    }
}

void* LE_LL_Add(void* list, void* value) {
    struct LinkedList_void* ll = list;
    while (ll->next) ll = ll->next;
    struct LinkedList_void* entry = malloc(sizeof(struct LinkedList_void));
    ll->next = entry;
    entry->next = NULL;
    entry->prev = ll;
    entry->frst = ll->frst;
    entry->value = value;
    return entry;
}

void LE_LL_Remove(void* list, void* value) {
    struct LinkedList_void* ll = list;
    while (ll) {
        if (ll->value == value) {
            struct LinkedList_void* next = ll->next;
            struct LinkedList_void* prev = ll->prev;
            free(ll);
            next->prev = prev;
            prev->next = next;
            ll = next;
        }
        else ll = ll->next;
    }
}

int LE_LL_Size(void* list) {
    struct LinkedList_void* ll = list;
    int i = 0;
    while (ll->next) {
        ll = ll->next;
        i++;
    }
    return i;
}

void* LE_LL_Get(void* list, int index) {
    struct LinkedList_void* ll = list;
    for (int i = 0; i <= index; i++) {
        if (!ll) return NULL;
        ll = ll->next;
    }
    return ll->value;
}
