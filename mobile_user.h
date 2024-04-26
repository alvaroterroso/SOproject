//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#ifndef mobile_user
#define mobile_user

#include "log.h"
#include "system_manager.h"
//mutex mobile user count
pthread_mutex_t usercount_mutex;

pid_t process;

//struct to message queue
typedef struct {
  /* Use the pid as the identifier */
  long id;
  /* Notifications about plafond */
  char *msg;
} plafond_msg;



#endif
