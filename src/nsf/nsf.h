#ifndef SMBR_NSF_H
#define SMBR_NSF_H

#include <stdbool.h>

typedef struct NSFHandle NSFHandle;

NSFHandle* nsf_load(const unsigned char* buf, unsigned int len);
void nsf_select(NSFHandle* handle, int song);
void nsf_play(NSFHandle* handle);
bool nsf_is_playing(NSFHandle* handle);
void nsf_stop(NSFHandle* handle);
void nsf_resume(NSFHandle* handle);
void nsf_dispose(NSFHandle* handle);

#endif
