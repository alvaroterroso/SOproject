//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#ifndef system_manager
#define system_manager

#include "shared_mem.h"
#include "mobile_user.h"
#include "backoffice_user.h"
#include "log.h"

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

#endif
