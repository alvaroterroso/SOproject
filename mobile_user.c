//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#include "mobile_user.h"
int fd_write;


pthread_mutex_t request_number = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t contorl_write = PTHREAD_MUTEX_INITIALIZER;

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

	signal(SIGINT, signal_handler);//HANDLE CTRL-C

	full=0;

	//register message
	memset(log_msg,0,sizeof(log_msg));
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
		if (pthread_create(&worker[i], NULL, send_data, (void*)thread) != 0){
			printf("CANNOT CREATE WORKER THREAD\n");
			exit(1);
		}
	}


	int mq = get_msg_id();
	printf("MESSAGE QUEUE ID: %d\n", mq);
	plafond_msg plafond;
	while(1){
		//only receives messages that belongs to this process
		printf("waiting for message queue.[%d]\n",(int)getpid());
		if (msgrcv(mq, &plafond, sizeof(plafond) - sizeof(long), (long)new_mobile_user.id, 0) == -1){ 
			perror("Error receiving message");
			// Adicionar tratamento de erro adequado
		} else {
			printf("MENSAGEM RECEBIDA: %s\n", plafond.msg);
			// Processar a mensagem recebida
		}

	}

	clear_resources();

	return 0;

}

void clear_resources(){
	for(int i=0; i<3; i++){//nao deve ser aqui o sitio mas para ja ta
		pthread_cancel(worker[i]);
		pthread_join(worker[i],NULL);
	}
	pthread_mutex_destroy(&request_number);
	pthread_mutex_destroy(&contorl_write);
	sem_unlink("full");
	sem_destroy(sem_full);

}

void signal_handler(){
	clear_resources();
	exit(0);
}

void *send_data(void* arg) {
    ThreadArg *thread = (ThreadArg*) arg;

    while (1) {
        pthread_mutex_lock(&request_number); // Espera pelo semáforo para manipular o número de requisições
		memset(log_msg,0,sizeof(log_msg));
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

		fflush(stdout);
		if(log_msg!=NULL){
			pthread_mutex_lock(&contorl_write);
			ssize_t bytes_written  = write(fd_write, log_msg, strlen(log_msg) + 1);  // Envia a mensagem
			if (bytes_written == -1) {
				printf("ERRO A ENVIAR A MENSAGEM PARA O PIPE\n");
			}else printf("MENSAGEM A ENVIAR PELO MOBILE USER : %s\n", log_msg);
			pthread_mutex_unlock(&contorl_write);
			sleep(thread->interval); // Aguarda pelo intervalo especificado antes de enviar a próxima mensagem
		}
    }
	return NULL;
}

int get_msg_id(){
    int msqid;
    FILE *fp = fopen(MSQ_FILE, "r");
    if (fp == NULL) {
        perror("Error opening file");
        exit(1);
    }
    if (fscanf(fp, "%d", &msqid) != 1) {
        perror("Error reading msqid from file");
        exit(1);
    }
    fclose(fp);
    return msqid;
}