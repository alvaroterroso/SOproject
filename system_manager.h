//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#ifndef system_manager
#define system_manager

#include "shared_mem.h"
#include "mobile_user.h"
#include "backoffice_user.h"
#include "log.h"


#define PLA_80 "ALERT 80%% TRIGGERED"
#define PLA_90 "ALERT 90%% TRIGGERED"
#define PLA_100 "ALERT 100%% TRIGGERED, REMOVING USER"
//identifier of message queue
int mqid;
//arranjar maneira de por esta variavel -> config.max_mobile_user
//struct to message queue
typedef struct {
  /* Use the pid as the identifier */
  long id;
  /* Notifications about plafond */
  char msg[MAX_STRING_SIZE];
} plafond_msg;

plafond_msg monitor;


int fd_read_user,fd_read_back;

int **pipes;

int flag;//só deixar os pedidos de dados serem feitos depois do login ter sido registado

bool validate_config(char * filename);
void init_prog();
void *sender_function(void *arg);
void *receiver_function(void *arg);
void free_shared();
void auth_request_manager();
void monitor_engine();
void create_proc();
void signal_handler();
void create_pipes(char * named);
void login_mobile(int id, int plafond);
void auth_mobile(int id, char type[MAX_STRING_SIZE], int amount);
void create_msq();
void create_unnamed_pipes();
void create_autho_engines();
void read_from_unnamed(int i);
void manage_auth(char *buf);
void add_queue(queue **head, const char *message, pthread_mutex_t sem) ;
char *rem_queue(queue **head, pthread_mutex_t sem);
int is_empty(queue *head, pthread_mutex_t sem,char tipo[MAX_STRING_SIZE]);	
queue * write_unnamed(queue *q_some, pthread_mutex_t mut, int i);
void print_queue(queue *head, pthread_mutex_t sem,char tipo[MAX_STRING_SIZE]);
void process_queue_item(queue **q, pthread_mutex_t mut);
int countUsers(queue *head, pthread_mutex_t sem);
void add_stats(char *msg);

#endif