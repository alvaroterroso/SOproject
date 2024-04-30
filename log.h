//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#ifndef log
#define log

#include "shared_mem.h"


int log_message(char* message);
void addUser(users_ **head, int id_, int plaf);
users_* searchUser(users_ *head, int id_);
int removeUser(users_ **head, int id_); 
int verificaS(const char *str);
#endif
