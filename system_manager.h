#ifndef system_manager
#define system_manager

#include "shared_mem.h"

bool validate_config(char * filename);

void init_prog();

void *sender_function(void *arg);

void *receiver_function(void *arg);


#endif
