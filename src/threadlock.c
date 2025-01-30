#include "threadlock.h"

#include <pthread.h>
#include <stdbool.h>

struct {
    bool inited;
    union {
        pthread_mutex_t mutex;
        pthread_rwlock_t rwlock;
    };
} threadlocks[THREADLOCK_NUM_IDS];

#define GET_HANDLER(handler) ({                                                           \
    if (!threadlocks[id].inited) pthread_##handler##_init(&threadlocks[id].handler, NULL); \
    threadlocks[id].inited = true;                                                          \
    &threadlocks[id].handler;                                                                \
})

void threadlock_mutex_lock(enum ThreadlockID id) {
    pthread_mutex_lock(GET_HANDLER(mutex));
}

void threadlock_mutex_unlock(enum ThreadlockID id) {
    pthread_mutex_unlock(GET_HANDLER(mutex));
}

void threadlock_io_read_lock(enum ThreadlockID id) {
    pthread_rwlock_rdlock(GET_HANDLER(rwlock));
}

void threadlock_io_write_lock(enum ThreadlockID id) {
    pthread_rwlock_wrlock(GET_HANDLER(rwlock));
}

void threadlock_io_unlock(enum ThreadlockID id) {
    pthread_rwlock_unlock(GET_HANDLER(rwlock));
}