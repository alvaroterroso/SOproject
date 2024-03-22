#ifndef system_manager
#define system_manager

#include "shared_mem.h"

bool validate_config(char * filename);

void init_prog();

void *sender_function(void *arg);

void *receiver_function(void *arg);

void free_shared(int shm_id);

void auth_request_manager();

void monitor_engine();

void create_proc();

#endif
