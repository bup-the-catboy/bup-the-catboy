#ifndef BTCB_THREADLOCK_H
#define BTCB_THREADLOCK_H

enum ThreadlockID {
    THREADLOCK_LEVEL_UPDATE,

    THREADLOCK_NUM_IDS
};

void threadlock_mutex_lock(enum ThreadlockID id);
void threadlock_mutex_unlock(enum ThreadlockID id);

void threadlock_io_read_lock(enum ThreadlockID id);
void threadlock_io_write_lock(enum ThreadlockID id);
void threadlock_io_unlock(enum ThreadlockID id);

#endif