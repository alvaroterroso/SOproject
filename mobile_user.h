//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#ifndef mobile_user
#define mobile_user
#define MAX_SIZE 256
#include "log.h"
#include "system_manager.h"
//mutex mobile user count
mobile_user_struct new_mobile_user;
sem_t * mens_pipe, * request_number, *sem_full;
pid_t filhos[3];
void clear_resources();
void send_data(int interval, char *tipo);

#endif


