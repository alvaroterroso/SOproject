//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#include "mobile_user.h"
int fd_write;


int main(int argc, char **argv){
	if(argc < 7){
		printf("Wrong input command!\nUsage: mobile_user <plafond inicial> "
		       "<número de pedidos de autorização> <intervalo VIDEO> "
		       "<intervalo MUSIC> <intervalo SOCIAL> <dados a reservar>\n");
		exit(1);
	}
	log_message("MOBILE USER IS RUNNING");

	mobile_user_struct *new_mobile_user = malloc(sizeof(mobile_user_struct));
	
	for(int i = 1; i < 7; i++){
		if(atoi(argv[i]) < 1){
			printf("INPUT ERROR!\n%s must be > 0", argv[i]);
			exit(1);
		}
	}

	new_mobile_user->init_plafond = atoi(argv[1]);
	new_mobile_user->auth_request_number =atoi(argv[2]);
	new_mobile_user->video_interval = atoi(argv[3]);
	new_mobile_user->music_interval = atoi(argv[4]);
	new_mobile_user->social_interval = atoi(argv[5]);
	new_mobile_user->to_reserve_data = atoi(argv[6]);

	new_mobile_user->id = (int)getpid();

	//register message
	snprintf(log_msg, sizeof(log_msg), "%d#%d",new_mobile_user->id, new_mobile_user->init_plafond);
	if((fd_write = open(USER_PIPE, O_WRONLY))<0){
		log_message("CANNOT OPEN PIPE FOR WRITING");
		clear_resources();
		exit(1);
	}
	write(fd_write, log_msg, sizeof(log_msg));

	if (sem_init(mens_pipe, 0, 1) == -1) { // O terceiro argumento é o valor inicial do semáforo
        log_message("ERRO A CRIAR SEMAFORO NO MOBILE USER.");
        exit(1);
    }
	/*
	int intervalos[3] = {new_mobile_user->video_interval, new_mobile_user->music_interval, new_mobile_user->social_interval};
	char *tipo[3] = {}
	for(int i=0; i<3; i++){
		filhos[i]=fork();
		if(filhos[i]==0){
			send_data(intervalos[i], )
		}
	}
	*/

	mqid = msgget(IPC_PRIVATE,0777);
	plafond_msg plafond;
	while(1){
		//only receives messages that belongs to this process
		msgrcv(mqid,&plafond,sizeof(plafond)-sizeof(long),(long)new_mobile_user->id,0);
		/*
		
		Analyse what to do here.

		
		*/
	}


	free(new_mobile_user);
	return 0;

}


void clear_resources(){

}

void send_data(int interval){

}
