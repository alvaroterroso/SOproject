//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#ifndef system_manager
#define system_manager

#include "shared_mem.h"
#include "mobile_user.h"
#include "backoffice_user.h"
#include "log.h"

//identifier of message queue
int mqid;
int fd_read, fd_write;//arranjar maneira de por esta variavel -> config.max_mobile_user
//struct to message queue
typedef struct {
  /* Use the pid as the identifier */
  long id;
  /* Notifications about plafond */
  char *msg;
} plafond_msg;

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

#endif
