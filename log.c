//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#include "log.h" 
#include "shared_mem.h"
#include "system_manager.h"
#include <time.h>


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
    pthread_mutex_lock(&log_mutex);

    FILE* log_file = fopen(FILENAME, "a");
    if (!log_file) {
        perror("Failed to open log file");
        pthread_mutex_unlock(&log_mutex);//free mutex to avoid deadlocks
		return 1;
        exit(EXIT_FAILURE);
    }

	//Writes message with timestamp captured on the top
    fprintf(log_file, "%s - %s\n", buffer, message);
    //Also prints on the console
    printf("%s - %s\n", buffer, message);

    fclose(log_file);
    pthread_mutex_unlock(&log_mutex);
	return 0;
}

void addUser(users_ **head, int id_, int plaf) {				
    users_ *new_node = (users_ *)malloc(sizeof(users_));				//NO FIM DO PROGRAMA DAR FREE, OU NS ONDE É MSM :)
    if (new_node == NULL) {
        fprintf(stderr, "Erro ao alocar memória\n");
        return;
    }
    new_node->plafond = plaf;
	new_node->id=id_;
	sem_wait(sem_shared);
    new_node->next = *head;
    *head = new_node;
	sem_post(sem_shared);
}

users_* searchUser(users_ *head, int id_) {
	sem_wait(sem_shared);
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
}

int removeUser(users_ **head, int id_) {
	sem_wait(sem_shared);
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


//FUNÇÕES RESPONSAVEIS POR FAZER OPERAÇÕES NAS FILAS DE VIDEO E OTHER

void add_queue(queue **head, const char *message, pthread_mutex_t sem) {
    // Criar um novo nó
    queue *newNode = malloc(sizeof(queue));
    if (newNode == NULL) {
        fprintf(stderr, "Falha na alocação de memória\n");
        return;
    }
    strncpy(newNode->message, message, MAX_STRING_SIZE);
    newNode->message[MAX_STRING_SIZE - 1] = '\0';  // Garantir terminação nula
    newNode->next = NULL;

	pthread_mutex_lock(&sem);
    if (*head == NULL) {
        // Se a fila está vazia, o novo nó se torna o cabeçalho
        *head = newNode;
		pthread_mutex_unlock(&sem);
		return;
    } else {
        // Caso contrário, encontrar o último nó
        queue *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
		pthread_mutex_unlock(&sem);
		return;
    }
}

char *rem_queue(queue **head, pthread_mutex_t sem){
	pthread_mutex_lock(&sem);
    if (*head == NULL) {
        fprintf(stderr, "A fila está vazia\n");
		pthread_mutex_unlock(&sem);
        return NULL;
    }

    queue *temp = *head;
    static char message[MAX_STRING_SIZE];  // Static para retornar localmente
    strncpy(message, temp->message, MAX_STRING_SIZE);

    *head = temp->next;
	pthread_mutex_unlock(&sem);
    free(temp);

    return message;
}

int is_empty(queue *head, pthread_mutex_t sem) {	//1 se está vazia, 0 tem elementos
	pthread_mutex_lock(&sem);
	if(head==NULL){
		pthread_mutex_unlock(&sem);
		return 1;
	}else{
		pthread_mutex_unlock(&sem);
		return 0;
	}
}



void write_unnamed(int **pipes){
	char msg[MAX_STRING_SIZE];
	char *temp = rem_queue(&q_video, mut_video);
	if (temp) {
		strncpy(msg, temp, MAX_STRING_SIZE);
		msg[MAX_STRING_SIZE - 1] = '\0'; // Garantir terminação nula
		free(temp); // Supondo que você deva liberar a memória (depende de como rem_queue() aloca a string)
	}

	sem_wait(sem_read_count);
	for(int i=0; i<config.max_auth_servers; i++){
		if(shared->read_count_shared[i]==0){
			shared->read_count_shared[i]=1;
			sem_post(sem_read_count);
			ssize_t num_written = write(pipes[i][1], msg, sizeof(msg));
			if (num_written == -1) {
				log_message("ERROR WRITING ON UNNAMED PIPE.");
				run=0;
				free_shared();
				exit(1);
			}
			return;
			
		}
	}
}