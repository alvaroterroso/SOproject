//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#ifndef system_manager
#define system_manager

#include "shared_mem.h"
#include "mobile_user.h"
#include "backoffice_user.h"
#include "log.h"


#define PLA_80 "ALERT TRIGGERED: YOU HAVE USED 80%% OF YOUR PLAFOND"
#define PLA_90 "ALERT TRIGGERED: YOU HAVE USED 90%% OF YOUR PLAFOND"
#define PLA_100 "ALERT TRIGGERED: YOU HAVE USED 100%% OF YOUR PLAFOND, REMOVING..."
#define MOB_FULL "MOBILE USER´S LIST IS FULL, CLOSING..."
#define data "1#data_stats"
#define reset "1#reset"

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

int flag;//flag para saber que a mensagem a enviar é de MOB_FULL

int mq;

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
char *rem_queue(queue **head, pthread_mutex_t sem,int type);
int is_empty(queue *head, pthread_mutex_t sem,char tipo[MAX_STRING_SIZE]);	
queue * write_unnamed(queue *q_some, pthread_mutex_t mut, int i,int type);
void print_queue(queue *head, pthread_mutex_t sem,char tipo[MAX_STRING_SIZE]);
void process_queue_item(queue **q, pthread_mutex_t mut,int type);
int countUsers(queue *head, pthread_mutex_t sem);
void add_stats(char *msg);
int get_msg_id();
void *statics_function();
void *plafond_function();
void pedriodic_data();
void process_message_from_pipe(char * msg);
void check_full(queue *head, pthread_mutex_t sem);
long long current_time_millis();
#endif