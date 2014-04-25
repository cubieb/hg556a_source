#ifndef PSISEMA_H
#define PSISEMA_H

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

struct sembuf lock[2] = {{0,0,0},{0, 1, SEM_UNDO}}; 
struct sembuf unlock[1] = {{0, -1, IPC_NOWAIT|SEM_UNDO}}; // set SMrdn
int      psi_semid = -1; // ipc semaphore id
long KEY_ID = 0x19010457;
#endif

