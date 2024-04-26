//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

#include "mobile_user.h"

usercount_mutex = PTHREAD_MUTEX_INITIALIZER;

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

	process = getpid();

	pthread_mutex_lock(&usercount_mutex);
	mobile_user_count++;
	pthread_mutex_unlock(&usercount_mutex);

	snprintf(log_msg, sizeof(log_msg), "Mobile user nº %d data as been saved in mobile_user struct", mobile_user_count);
	log_message(log_msg);

	
	/*
	
		//send the data through threads to the named pipe here
	
	*/

	mqid_mobile = msgget(IPC_PRIVATE,0777);
	plafond_msg plafond;
	while(1){
		//only receives messages that belongs to this process
		msgrcv(mqid_mobile,&plafond,sizeof(plafond)-sizeof(long),(long)process,0);
		/*
		
		Analyse what to do here.

		
		*/
	}


	free(new_mobile_user);
	return 0;

}

