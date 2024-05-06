//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#include "log.h" 
#include "shared_mem.h"
#include "system_manager.h"
#include <time.h>

//mutex log

// Function to log messages
int log_message(char* message) {
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);//fetch current time for more accurate output
																//if not, the time would be affected by the mutex pause

	//Get the mutex to write to the log file
    sem_wait(log_mutex);

    FILE* log_file = fopen(FILENAME, "a");
    if (!log_file) {
        perror("Failed to open log file");
        sem_post(log_mutex);//free mutex to avoid deadlocks
		return 1;
        exit(EXIT_FAILURE);
    }

	//Writes message with timestamp captured on the top
    fprintf(log_file, "%s - %s\n", buffer, message);
    //Also prints on the console
    printf("%s - %s\n", buffer, message);

    fclose(log_file);
    sem_post(log_mutex);
	return 0;
}

void addUser(int id_, int plaf) {
	/**/	
	int i = 0;
	sem_post(sem_shared);
	while(shared->user_array[i][0] != -1){
		i++;
	}
	shared->user_array[i][0] = id_;
	shared->user_array[i][1] = plaf;
	sem_wait(sem_shared);
	
	return;
	// A -> COMENTAR O RESTO
	/*
	users_ *new_node = (users_ *)malloc(sizeof(users_));				//NO FIM DO PROGRAMA DAR FREE, OU NS ONDE É MSM :)
    if (new_node == NULL) {
        fprintf(stderr, "Erro ao alocar memória\n");
        return;
    }
    new_node->plafond = plaf;
	new_node->plafond_ini = plaf;
	new_node->id=id_;
	sem_wait(sem_shared);
    new_node->next = *head;
    *head = new_node;
	sem_post(sem_shared);

	return;
	*/

}

int searchUser(int id_) { // A -> har porque é a única forma de passar o id e user na mesma função 
	sem_wait(sem_shared);
	int i = 0;
	while(shared->user_array[i][0] != id_){
		i++;
	}
	if(shared->user_array[i][0] != -1)
	{
		return i;
	}
	return -1;

	// A-> COMENTAR O RESTO 
	/*
	 users_ *current = head;
    while (current != NULL) {
        if (current->id == id_) {
			sem_post(sem_shared);
            return current;
        }
        current = current->next;
    }
	sem_post(sem_shared);
    return NULL;  // Não encontrado
	*/
}

int removeUser(int id_) {
	int i = 0;
	sem_wait(sem_shared);
	while(shared->user_array[i][0] != id_){
		i++;
	}
	if(shared->user_array[i][0] != -1){
		shared->user_array[i][0] = -1;
		shared->user_array[i][1] = 0;
		return 1;// A -> RETURN 1 IF FOUND
	} 
	return 0;// A-> RETURN 0 IF NOT FOUND

	// ############ A -> APAGAR O RESTO PARA BAIXO ...

	/*
	users_ *current = *head;
    users_ *previous = NULL;

    while (current != NULL) {
        if (current->id == id_) {
            if (previous == NULL) {  // Remover o primeiro nó
                *head = current->next;
            } else {  // Remover nó que não é o primeiro
                previous->next = current->next;
            }
            free(current);
			sem_post(sem_shared);
            return 1;  // Removido com sucesso
        }
        previous = current;
        current = current->next;
    }
	sem_post(sem_shared);
    return 0;  // Não encontrado
	*/
}

int verificaS(const char *str) {
    bool contemLetras = false;
    bool contemNumeros = false;

    // Verifica cada caractere da string
    for (int i = 0; str[i] != '\0'; i++) {
        if (isalpha(str[i])) {
            contemLetras = true;
        } else if (isdigit(str[i])) {
            contemNumeros = true;
        }
    }

    // Retorna o código correspondente ao tipo de string
    if (contemLetras && contemNumeros) {
        return 3; // Contém letras e números
    } else if (contemLetras) {
        return 1; // Contém apenas letras
    } else if (contemNumeros) {
        return 2; // Contém apenas números
    } else {
        return 0; // String vazia ou inválida
    }
}

