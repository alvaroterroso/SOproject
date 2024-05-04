//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#include "mobile_user.h"
int fd_write;
int full=0;


int main(int argc, char **argv){
	if(argc < 7){
		printf("Wrong input command!\nUsage: mobile_user <plafond inicial> "
		       "<número de pedidos de autorização> <intervalo VIDEO> "
		       "<intervalo MUSIC> <intervalo SOCIAL> <dados a reservar>\n");
		exit(1);
	}
	log_message("MOBILE USER IS RUNNING");

	
	for(int i = 1; i < 7; i++){
		if(atoi(argv[i]) < 1){
			printf("INPUT ERROR!\n%s must be > 0", argv[i]);
			exit(1);
		}
	}

	new_mobile_user.init_plafond = atoi(argv[1]);
	new_mobile_user.auth_request_number =atoi(argv[2]);
	new_mobile_user.video_interval = atoi(argv[3]);
	new_mobile_user.music_interval = atoi(argv[4]);
	new_mobile_user.social_interval = atoi(argv[5]);
	new_mobile_user.to_reserve_data = atoi(argv[6]);
	new_mobile_user.id = (int)getpid();

	//register message
	snprintf(log_msg, sizeof(log_msg), "%d#%d",new_mobile_user.id, new_mobile_user.init_plafond);
	printf("mensagem mobile: %s\n",log_msg);
	if((fd_write = open(USER_PIPE, O_WRONLY))<0){
		log_message("CANNOT OPEN PIPE FOR WRITING");
		clear_resources();
		exit(1);
	}
	write(fd_write, log_msg, sizeof(log_msg));
	log_message("MENSAGEM ESCRITA");

	sem_unlink("escrita");
	mens_pipe = sem_open("escrita", O_CREAT|O_EXCL, 0777,1);

	sem_unlink("requests");
	request_number = sem_open("requests", O_CREAT|O_EXCL, 0777,1);

	sem_unlink("full");
	sem_full = sem_open("full", O_CREAT|O_EXCL, 0777,1);
	
	int intervalos[3] = {new_mobile_user.video_interval, new_mobile_user.music_interval, new_mobile_user.social_interval};
	char *tipo[3] = {"VIDEO", "MUSIC", "SOCIAL"};

	for(int i=0; i<3; i++){
		ThreadArg *thread = malloc(sizeof(ThreadArg));  // Alocação de memória
        if (thread == NULL) {
            perror("Failed to allocate memory for thread arguments");
            exit(1);
        }
        thread->interval = intervalos[i];
        thread->tipo = tipo[i];
		if (pthread_create(&worker[i], NULL, send_data, thread) != 0){
			log_message("CANNOT CREATE WORKER THREAD");
			exit(1);
		}
	}
	//por aqui um while que espera a variavel de condição mudar, ou seja, que tenha a certeza que o login foi efetuado
	// a variavel a verificar é =1 para dizer que o login foi bem feito e =2 se for rejeitado
	mqid = msgget(IPC_PRIVATE,0777);
	plafond_msg plafond;
	while(1){
		//only receives messages that belongs to this process
		msgrcv(mqid,&plafond,sizeof(plafond)-sizeof(long),(long)new_mobile_user.id,0);
		/*
		
		Analyse what to do here.

		PÕR UMA CONDIÇÃO DE, SE RECEBEU UMA MENSAGEM DE 100%, MUDAR A VARIAVEL FULL PARA IGUAL A 1

		
		*/
	}
	for(int i=0; i<3; i++){//nao deve ser aqui o sitio mas para ja ta
		pthread_join(worker[i],NULL);
	}

	return 0;

}


void clear_resources(){

}

void *send_data(void* arg){//VER SE POSSO USAR UMA SHARED VARIABLE PARA IR DECREMENTANDO O NUMERO DE REQUESTS
 	ThreadArg *thread = (ThreadArg*) arg;
	sem_wait(request_number);

	while(new_mobile_user.auth_request_number>0){
		sem_wait(sem_full);
		if(full!=1){
			sem_post(sem_full);
			--new_mobile_user.auth_request_number;
			sem_post(request_number);
			snprintf(log_msg, sizeof(log_msg), "%d#%s#%d",new_mobile_user.id, thread->tipo, new_mobile_user.to_reserve_data);
			sem_wait(mens_pipe);
			write(fd_write, log_msg, sizeof(log_msg));
			sem_post(mens_pipe);
			sleep(thread->interval);
		}else{
			sem_post(sem_full);
			break;
		}
	}
	sem_post(request_number);
	return NULL;
}
