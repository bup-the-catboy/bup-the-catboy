#ifndef BTCB_CONTEXT_H
#define BTCB_CONTEXT_H

void* context_create(int num, ...);
void* context_int  (const char* name, int   value);
void* context_float(const char* name, float value);
void* context_ptr  (const char* name, void* value);
void  context_destroy(void* ctx);

int   context_get_int  (void* ctx, const char* name);
float context_get_float(void* ctx, const char* name);
void* context_get_ptr  (void* ctx, const char* name);

#define NUMARGS(...) (sizeof((void*[]){ __VA_ARGS__ }) / sizeof(void*))
#define context_create(...) context_create(NUMARGS(__VA_ARGS__) __VA_OPT__(,) __VA_ARGS__)

#endif