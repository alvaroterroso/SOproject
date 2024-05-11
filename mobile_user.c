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

	run=1;

	son_mq = fork();
	if(son_mq == 0){
		read_mq();
		exit(0);
	}else if(son_mq == -1){
		printf("Error creating read_mq proc\n");
		clear_resources();
		exit(1);
	}

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
	

	if (pthread_create(&video_t, NULL, send_video, NULL) != 0){
		printf("CANNOT CREATE WORKER THREAD\n");
		exit(1);
	}
	if (pthread_create(&social_t, NULL, send_social, NULL) != 0){
		printf("CANNOT CREATE WORKER THREAD\n");
		exit(1);
	}
	if (pthread_create(&music_t, NULL, send_music, NULL) != 0){
		printf("CANNOT CREATE WORKER THREAD\n");
		exit(1);
	}
	
	pthread_join(music_t,NULL);
	pthread_join(video_t,NULL);
	pthread_join(social_t,NULL);

	run = 0;

	kill(son_mq, SIGTERM);  // Envia sinal SIGTERM para o processo filho
    wait(NULL);

	return 0;

}

void clear_resources(){
	pthread_mutex_destroy(&request_number);
	pthread_mutex_destroy(&contorl_write);

}

void signal_handler(){
	run=0;
	pthread_cancel(music_t);
	pthread_join(music_t,NULL);
	pthread_cancel(video_t);
	pthread_join(video_t,NULL);
	pthread_cancel(social_t);
	pthread_join(social_t,NULL);

	kill(son_mq, SIGTERM);  // Envia sinal SIGTERM para o processo filho
    wait(NULL);
	clear_resources();
	exit(0);
}

void *send_video() {
    while (run) {
        pthread_mutex_lock(&request_number); 
        if (new_mobile_user.auth_request_number <= 0) {
            pthread_mutex_unlock(&request_number); // Libera o semáforo se não há mais requisições 
            break; // Sai do loop se não há mais requisições 
        }
        --new_mobile_user.auth_request_number;
        pthread_mutex_unlock(&request_number); // Libera o semáforo após a modificação segura

        // Prepara a mensagem a ser enviada
        snprintf(log_msg, sizeof(log_msg), "%d#VIDEO#%d", new_mobile_user.id, new_mobile_user.to_reserve_data);

		if(log_msg!=NULL){
			pthread_mutex_lock(&contorl_write);
			ssize_t bytes_written  = write(fd_write, log_msg, strlen(log_msg) + 1);  // Envia a mensagem
			if (bytes_written == -1) {
				printf("ERRO A ENVIAR A MENSAGEM PARA O PIPE\n");
			}else printf("MENSAGEM A ENVIAR PELO MOBILE USER : %s\n", log_msg);
			pthread_mutex_unlock(&contorl_write);
		}
		printf("sleep video: %d\n",new_mobile_user.video_interval );
		sleep(new_mobile_user.video_interval);
    }
	return NULL;
}

void *send_music() {
    while (run) {
        pthread_mutex_lock(&request_number); 
        if (new_mobile_user.auth_request_number <= 0) {
            pthread_mutex_unlock(&request_number); // Libera o semáforo se não há mais requisições 
            break; // Sai do loop se não há mais requisições 
        }
        --new_mobile_user.auth_request_number;
        pthread_mutex_unlock(&request_number); // Libera o semáforo após a modificação segura

        // Prepara a mensagem a ser enviada
        snprintf(log_msg, sizeof(log_msg), "%d#MUSIC#%d", new_mobile_user.id, new_mobile_user.to_reserve_data);

		if(log_msg!=NULL){
			pthread_mutex_lock(&contorl_write);
			ssize_t bytes_written  = write(fd_write, log_msg, strlen(log_msg) + 1);  // Envia a mensagem
			if (bytes_written == -1) {
				printf("ERRO A ENVIAR A MENSAGEM PARA O PIPE\n");
			}else printf("MENSAGEM A ENVIAR PELO MOBILE USER : %s\n", log_msg);
			pthread_mutex_unlock(&contorl_write);
		}
		printf("sleep music: %d\n",new_mobile_user.music_interval );
		sleep(new_mobile_user.music_interval);
    }
	return NULL;
}

void *send_social() {
    while (run) {
        pthread_mutex_lock(&request_number); 
        if (new_mobile_user.auth_request_number <= 0) {
            pthread_mutex_unlock(&request_number); // Libera o semáforo se não há mais requisições 
            break; // Sai do loop se não há mais requisições 
        }
        --new_mobile_user.auth_request_number;
        pthread_mutex_unlock(&request_number); // Libera o semáforo após a modificação segura

        // Prepara a mensagem a ser enviada
        snprintf(log_msg, sizeof(log_msg), "%d#SOCIAL#%d", new_mobile_user.id, new_mobile_user.to_reserve_data);

		if(log_msg!=NULL){
			pthread_mutex_lock(&contorl_write);
			ssize_t bytes_written  = write(fd_write, log_msg, strlen(log_msg) + 1);  // Envia a mensagem
			if (bytes_written == -1) {
				printf("ERRO A ENVIAR A MENSAGEM PARA O PIPE\n");
			}else printf("MENSAGEM A ENVIAR PELO MOBILE USER : %s\n", log_msg);
			pthread_mutex_unlock(&contorl_write);
		}
		printf("sleep social: %d\n",new_mobile_user.social_interval );
		sleep(new_mobile_user.social_interval);
    }
	return NULL;
}


void read_mq(){
	int mq = get_msg_id();

	plafond_msg plafond;
	while(run){

		if (msgrcv(mq, &plafond, sizeof(plafond) - sizeof(long), (long)new_mobile_user.id, 0) == -1){ 
			printf("System Manager Closing...\n");
			run = 0;
		} else {
			if(strcmp(plafond.msg, PLA_80)==0 || strcmp(plafond.msg, PLA_90)==0){
				printf("MESSAGE FROM SYSTEM MANAGER: %s\n",plafond.msg);
			}else if(strcmp(plafond.msg, PLA_100)==0 || strcmp(plafond.msg, MOB_FULL) == 0){
				printf("MESSAGE FROM SYSTEM MANAGER: %s\n",plafond.msg);
				run = 0;
			}
		}

	}
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