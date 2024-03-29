//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#ifndef shared_mem   /* Include guard */
#define shared_mem

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>

#define PIPENAME_1 "USER_PIPE"

#define PIPENAME_2 "BACK_PIPE"

#define MAX_STRING_SIZE 256

#define FILENAME "log.txt"

typedef struct config_struct{
	int queue_slot_number;
	int max_auth_servers;
	int auth_proc_time; //ms
	int max_video_wait; //ms
	int max_others_wait; //ms
}config_struct;

typedef struct mobile_user_struct{
	int init_plafond;
	int auth_request_number;
	int video_interval;
	int music_interval;
	int social_interval;
	int to_reserve_data;
}mobile_user_struct;

//mobile_user count
int mobile_user_count;

//inicializar shared memorym
int shm_id;

//config filename
char filename[MAX_STRING_SIZE];

//log file
char log_msg[MAX_STRING_SIZE];

//config file
config_struct config;

//threads
pthread_t receiver_thread, sender_thread;

//process
pid_t auth_request_manager_pid, monitor_engine_pid, system_manager_pid;

//mutex log
pthread_mutex_t log_mutex; // Extern keyword added here

#endif

