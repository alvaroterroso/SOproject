//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#ifndef shared_mem   /* Include guard */
#define shared_mem

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <ctype.h>

#define USER_PIPE "/tmp/USER_PIPE"
#define BACK_PIPE "/tmp/BACK_PIPE"

#define MAX_STRING_SIZE 256

#define FILENAME "log.txt"

//A FAZER AMANHA
//1-> IMPLEMETNRA AS FUNÇOES DE INSERIR, REMOVER, VER SE ESTÁ VAZIA 				X
//2-> IMPLEMENTAR ESSAS FUNÇÕES NA RECEIVER THREAD									X
//3-> NA FUNÇÃO SENDER, FAZER O ALGORITMO DE DAR A PRIORIDADE A VIDEO QUEUE        	Falta implementar as variaveis de condição
//4-> GARANTIR QUE A SENDER ESTÁ A COMUNICAR BEM COM OS UNNAMED PIPES
//5-> SEMPRE QUE RETIRARMOS PLAFONS, VER QUANTO FALTA PARA ESGOTAR, OU SE JA ESGOTOU, PARA DEPOIS ENVIAR UMA MQ


typedef struct config_struct{
	int max_mobile_user;
	int queue_slot_number;
	int max_auth_servers;
	int auth_proc_time; //ms
	int max_video_wait; //ms
	int max_others_wait; //ms
}config_struct;

typedef struct mobile_user_struct{
	int id; //getpid()
	int init_plafond;
	int auth_request_number;
	int video_interval;
	int music_interval;
	int social_interval;
	int to_reserve_data;
}mobile_user_struct;

//a nossa shared memory
typedef struct users_{
	int id;
	int plafond;
	struct users_ *next;
}users_;

typedef struct shm{
	users_ *users;		     //lista dos mobile users criados
	int * read_count_shared; //array dos unnamed pipes disponiveis
	int mobile_users;		 //numero de mobile users registados
}shm;

sem_t *sem_plafond; // semaforo para lidar com a a variavel plafond dos user

shm *shared; 

sem_t *sem_shared; //semaforo para lidar com a fila da shared memory
sem_t *sem_userscount; //semaforo para lidar com o usercount da shared memory
sem_t *sem_read_count; //semaforo para lidar com o read_count da shared memory

sem_t *sem_controlar; //semaforo que só deixa a sender verificar se ha mensagens para ler ( só desbloqueia quando alguem envia para a fila)

typedef struct queue{
	char message[MAX_STRING_SIZE];
	struct queue *next;
}queue;

queue *q_video;
queue *q_other;

int run;

//variaveis para as estatisticas	0->total data 	1->auth reqs
int video[2];
int music[2];
int social[2];

//inicializar shared memory
int shm_id;

//config filename
char filename[MAX_STRING_SIZE];

//log file
char log_msg[MAX_STRING_SIZE]; // fazer antes malloc para dar free no fim, so ainda n fiz isso pq ns onde dar o free

//config file
config_struct config;

//threads
pthread_t receiver_thread, sender_thread;

//process
pid_t auth_request_manager_pid, monitor_engine_pid, system_manager_pid;

//mutex log
pthread_mutex_t log_mutex; // Extern keyword added here

//criaç
pid_t *autho_engines_pid;

#endif