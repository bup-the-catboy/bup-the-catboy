#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

struct ContextItem {
    const char* name;
    void* value;
    struct ContextItem* next;
};

void* context_create(int num, ...) {
    struct ContextItem *ctx, *head = calloc(sizeof(struct ContextItem), 1);
    ctx = head;
    va_list args;
    va_start(args, num);
    for (int i = 0; i < num; i++) {
        struct ContextItem* item = va_arg(args, struct ContextItem*);
        head->name = item->name;
        head->value = item->value;
        head->next = calloc(sizeof(struct ContextItem), 1);
        head = head->next;
        free(item);
    }
    va_end(args);
    return ctx;
}

static void* context_create_item(const char* name, void* value, int size) {
    struct ContextItem* item = malloc(sizeof(struct ContextItem));
    void* ptr = malloc(size);
    memcpy(ptr, value, size);
    item->name = name;
    item->value = ptr;
    return item;
}

static void* context_find(struct ContextItem* list, const char* name) {
    struct ContextItem* curr = list;
    while (curr) {
        if (curr->name && strcmp(curr->name, name) == 0) return curr->value;
        curr = curr->next;
    }
    return NULL;
}

void* context_int(const char* name, int value) {
    return context_create_item(name, &value, sizeof(int));
}

void* context_float(const char* name, float value) {
    return context_create_item(name, &value, sizeof(float));
}

void* context_ptr  (const char* name, void* value) {
    return context_create_item(name, &value, sizeof(void*));
}

int context_get_int(void* ctx, const char* name) {
    int* ptr = context_find(ctx, name);
    return ptr ? *ptr : 0;
}

float context_get_float(void* ctx, const char* name) {
    float* ptr = context_find(ctx, name);
    return ptr ? *ptr : 0;
}

void* context_get_ptr(void* ctx, const char* name) {
    void** ptr = context_find(ctx, name);
    return ptr ? *ptr : 0;
}

void context_destroy(void* ctx) {
    struct ContextItem* curr = ctx;
    while (curr) {
        struct ContextItem* next = curr->next;
        if (curr->value) free(curr->value);
        free(curr);
        curr = next;
    }
}
