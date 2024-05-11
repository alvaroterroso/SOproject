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
pthread_t social_t;
pthread_t video_t;
pthread_t music_t;
pid_t son_mq;

void clear_resources();
void *send_data(void* arg);
int get_msg_id();
void read_mq();

void *send_social();
void *send_video();
void *send_music();


#endif


