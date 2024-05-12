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
#include <time.h> 
#include <sys/time.h>

#define USER_PIPE "/tmp/USER_PIPE"
#define BACK_PIPE "/tmp/BACK_PIPE"
#define MSQ_FILE "msq_id.txt"

#define MAX_STRING_SIZE 215

#define FILENAME "log.txt"

//TODO
//1-> Lidar com as mensagens lidas na mq no  mobile user
//2-> Fazer a cena de dividir as strings quando tamos a ler no USER PIPE
//3-> Ver melhor as funções de fechar po sistema, ter a certeza que tamos a fechar todos os processos e assim
//4-> FAZER A CENA DE ARRANJAR UM PIPE EXTRA
//5-> VER SE FAÇO AQUILO DE SO ESPERAR UM TEMPO X NA FILA
//6-> ARRANJAR MANEIRA DE ENVIAR PELA MQ QUE O UTILIZADOR NAO CONSEGUIU LOGAR(CASO ACONTEÇa)

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

typedef struct users_{//COMENTAR USERS DA SHARED MEM
	int id;
	float plafond;
	float plafond_ini;
}users_;

typedef struct stats_struct{
	int total_video, video_req;
	int total_social, social_req;
	int total_music, music_req;
}stats_struct;

typedef struct shm{
	users_ *user_array; 		//array de users
	int * read_count_shared; //array dos unnamed pipes disponiveis
	int mobile_users;		 //numero de mobile users registados
	int flag; 				//saber quando o mobile user nao conseguiu dar login pela fila estar cheia
	bool run;
	stats_struct stats;      //stats do backoffice_user
}shm;

shm *shared; 

sem_t *sem_plafond; // semaforo para lidar com a shared memory


sem_t *sem_shared; //semaforo para lidar com a fila da shared memory
sem_t *sem_userscount; //semaforo para lidar com o usercount da shared memory
sem_t *sem_read_count; //semaforo para lidar com o read_count da shared memory
sem_t * log_mutex;
//sem_t *sem_controlar; //semaforo que só deixa a sender verificar se ha mensagens para ler ( só desbloqueia quando alguem envia para a fila)
sem_t *sem_statics;	 //alterar o valor das estatisticas
sem_t *sem_monitor; //aletar o monitor quando tiver mensagens para ler
sem_t *sem_flag; 
sem_t *sem_go;
sem_t *sem_run;


typedef struct queue{
	char message[MAX_STRING_SIZE];
	long long time;
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
int shm_users;
int shm_readcount;

//config filename
char filename[MAX_STRING_SIZE];

//log file
char log_msg[MAX_STRING_SIZE]; // fazer antes malloc para dar free no fim, so ainda n fiz isso pq ns onde dar o free

//config file
config_struct config;

//threads
pthread_t receiver_thread, sender_thread, sender_thread, mobile_thread, back_thread; 

//process
pid_t auth_request_manager_pid, monitor_engine_pid, system_manager_pid;

//criaç
pid_t *autho_engines_pid;

#endif