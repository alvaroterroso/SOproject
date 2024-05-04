//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#ifndef log
#define log

#include "shared_mem.h"
#include "system_manager.h"

int log_message(char* message);
void addUser(users_ **head, int id_, int plaf);
users_ * searchUser(users_ *head, int id_);
int removeUser(users_ **head, int id_); 
int verificaS(const char *str);
void add_queue(queue **head, const char *message, pthread_mutex_t sem) ;
char *rem_queue(queue **head, pthread_mutex_t sem);
int is_empty(queue *head, pthread_mutex_t sem);	
void write_unnamed(int **pipes);
#endif
