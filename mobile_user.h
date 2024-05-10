//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#ifndef mobile_user
#define mobile_user
#define MAX_SIZE 256
#include "log.h"
#include "system_manager.h"
#include "shared_mem.h"
//mutex mobile user count
mobile_user_struct new_mobile_user;
sem_t *sem_full;
pthread_t worker[3];
void clear_resources();
void *send_data(void* arg);
int get_msg_id();

typedef struct {
    int interval;
    char *tipo;
} ThreadArg;

#endif


