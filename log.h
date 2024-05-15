//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#ifndef log
#define log

#include "shared_mem.h"
#include "system_manager.h"

int log_message(char* message);
void addUser(int id_, int plaf);
int searchUser(int id_);
int removeUser(int id_); 
int verificaS(const char *str);
int count_char_occurrences(const char *str, char character);
#endif