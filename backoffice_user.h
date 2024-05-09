//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#ifndef backoffice_user
#define backoffice_user
#include "system_manager.h"
#include "shared_mem.h"
#include "log.h"
#define data "1#data_stats"
#define reset "1#reset"
void ler_mq();
char input[MAX_STRING_SIZE];
#endif //BACKOFFICE_USER_H