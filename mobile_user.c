//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#include "mobile_user.h"
int fd_write;
int full=0;

pthread_mutex_t request_number = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv){
	if(argc < 7){
		printf("Wrong input command!\nUsage: mobile_user <plafond inicial> "
		       "<número de pedidos de autorização> <intervalo VIDEO> "
		       "<intervalo MUSIC> <intervalo SOCIAL> <dados a reservar>\n");
		exit(1);
	}

	printf("MOBILE USER IS RUNNING\n");

	
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
		printf("CANNOT OPEN PIPE FOR WRITING\n");
		clear_resources();
		exit(1);
	}

	write(fd_write, log_msg, sizeof(log_msg));//login message

	sem_unlink("full");
	sem_full = sem_open("full", O_CREAT|O_EXCL, 0777,1); //semaforo pq quem vai utilizar este semaforo tambem é o auth engine que é um processo
	
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
			printf("CANNOT CREATE WORKER THREAD\n");
			exit(1);
		}
	}
	//por aqui um while que espera a variavel de condição mudar, ou seja, que tenha a certeza que o login foi efetuado
	// a variavel a verificar é =1 para dizer que o login foi bem feito e =2 se for rejeitado
	mqid = msgget(IPC_PRIVATE,0777);
	plafond_msg plafond;
	while(1){
		//only receives messages that belongs to this process
		printf("waiting for message queue.\n");
		msgrcv(mqid,&plafond,sizeof(plafond)-sizeof(long),(long)new_mobile_user.id,0);

		printf("MENSAGEM RECEBIDA: %s\n", plafond.msg);
		/*
		
		Analyse what to do here.

		PÕR UMA CONDIÇÃO DE, SE RECEBEU UMA MENSAGEM DE 100%, MUDAR A VARIAVEL FULL PARA IGUAL A 1
		
		*/
	}

	clear_resources();

	return 0;

}

void clear_resources(){
	for(int i=0; i<3; i++){//nao deve ser aqui o sitio mas para ja ta
		pthread_join(worker[i],NULL);
	}
	pthread_mutex_destroy(&request_number);
	sem_unlink("full");
	sem_destroy(sem_full);

}

void *send_data(void* arg) {
    ThreadArg *thread = (ThreadArg*) arg;

    while (1) {
        pthread_mutex_lock(&request_number); // Espera pelo semáforo para manipular o número de requisições

        // Checa se ainda há requisições e se o sistema não está cheio
        if (new_mobile_user.auth_request_number <= 0 || full == 1) {
            pthread_mutex_unlock(&request_number); // Libera o semáforo se não há mais requisições ou se o sistema está cheio
            break; // Sai do loop se não há mais requisições ou se o sistema está cheio
        }

        // Decrementa o número de requisições de forma segura
        --new_mobile_user.auth_request_number;
        pthread_mutex_unlock(&request_number); // Libera o semáforo após a modificação segura

        // Prepara a mensagem a ser enviada
        snprintf(log_msg, sizeof(log_msg), "%d#%s#%d", new_mobile_user.id, thread->tipo, new_mobile_user.to_reserve_data);

        printf("MENSAGEM A ENVIAR PELO MOBILE USER : %s\n", log_msg);
		fflush(stdout);

        ssize_t bytes_written  = write(fd_write, log_msg, strlen(log_msg) + 1);  // Envia a mensagem

		if (bytes_written == -1) {
			printf("ERRO A ENVIAR A MENSAGEM PARA O PIPE\n");
		}

        sleep(thread->interval); // Aguarda pelo intervalo especificado antes de enviar a próxima mensagem
    }

    return NULL;
}