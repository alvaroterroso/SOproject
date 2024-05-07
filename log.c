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

// Função para adicionar um usuário
void addUser(int id_, int plaf) {
    sem_wait(sem_shared); // Deveria ser sem_wait para esperar antes de acessar
    int i = 0;
    while (shared->user_array[i].id != -1 && i < shared->mobile_users) {
        i++;
    }
    if (i < shared->mobile_users) { // Verifica se ainda há espaço no array
        shared->user_array[i].id = id_;
        shared->user_array[i].plafond = plaf;
        shared->user_array[i].plafond_ini = plaf; // Supondo inicialização do plafond inicial também
    }
    sem_post(sem_shared);
}

// Função para procurar um usuário pelo ID
int searchUser(int id_) {
    sem_wait(sem_shared);
    int i = 0;
    while (i < shared->mobile_users && shared->user_array[i].id != id_) {
        i++;
    }
    sem_post(sem_shared); // Libera o semáforo após o acesso
    if (i < shared->mobile_users && shared->user_array[i].id != -1) {
        return i;
    }
    return -1;
}

// Função para remover um usuário
int removeUser(int id_) {
    sem_wait(sem_shared);
    int i = 0;
    while (i < shared->mobile_users && shared->user_array[i].id != id_) {
        i++;
    }
    if (i < shared->mobile_users && shared->user_array[i].id != -1) {
        shared->user_array[i].id = -1; // Marca o ID como -1 para indicar remoção
        shared->user_array[i].plafond = -1; // Zera o plafond
        shared->user_array[i].plafond_ini = -1; // Zera o plafond inicial
        sem_post(sem_shared);
        return 1; // Usuário encontrado e removido
    } 
    sem_post(sem_shared);
    return 0; // Usuário não encontrado
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
