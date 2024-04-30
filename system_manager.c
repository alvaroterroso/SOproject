//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

/*
TODO:
PIPES -> CREATED, BUT NOT OPPENED
THREADS -> CREATED, BUT NOT FUNCTION

FIXES:
PROCESSES CREATED ON RIGHT PLACE
SIGNAL -> HANDLES CTRL-C, AND IGNORES WHILE SETUP

*/

#include "system_manager.h"

int main(int argc, char **argv){
	log_message("5G_AUTH_PLATFORM SIMULATOR STARTING");

	//ignore signal while inittilazing 

	signal(SIGINT, SIG_IGN);
  	signal(SIGTSTP, SIG_IGN);
  	signal(SIGUSR1, SIG_IGN);

	system_manager_pid = getpid();

	log_message("PROCESS SYSTEM_MANAGER CREATED");
	if (argc < 2) {
        fprintf(stderr, "Usage: %s <config-file-path> \n", argv[0]);
        return 1;
    }
	mobile_user_count = 0;
	strcpy(filename, argv[1]);
	if(!validate_config(filename)) exit(0);

	init_prog();

	signal(SIGINT, signal_handler);//HANDLE CTRL-C


	if(getpid() == system_manager_pid)log_message("SIMULATOR WAITING FOR LAST TASKS TO FINISH");
	free_shared();
	if(getpid() == system_manager_pid)log_message("5G_AUTH_PLATFORM SIMULATOR CLOSING\n"); //last message
	pthread_mutex_destroy(&log_mutex);
	return 0;
}

void signal_handler(){
	free_shared();
	kill(0, SIGTERM); // kills all processes
	exit(0);
}

void create_pipes(char * named){
	unlink(named);
	if ((mkfifo(named, O_CREAT|O_EXCL|0777)<0) && (errno != EEXIST)){
    	log_message("CANNOT CREATE NAMED PIPE -> EXITING\n");
    	exit(1);
  	}
	snprintf(log_msg, sizeof(log_msg), "%s CREATED", named);
	log_message(log_msg);
}

void free_shared(){
	unlink(BACK_PIPE);
	unlink(USER_PIPE);
	shmdt(&shm_id);
	shmctl(shm_id, IPC_RMID, NULL);
}

void init_prog(){
	int shm_size = sizeof(config_struct) + sizeof(users_) * config.max_mobile_user;
	if ((shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | IPC_EXCL | 0700)) < 1){
    	log_message("ERROR IN SHMGET");
    	exit(1);
  	}
	users=(users_*)shmat(shm_id, NULL, 0);
	if(users == (void*)-1){
		log_message("ERROR IN SHMAT");
		exit(1);
	}
	log_message("SHARED MEMORY IS ALLOCATED");

	mqid = msgget(IPC_PRIVATE,0777);
	if (mqid == -1) {
        log_message("ERROR IN MSGGET");
		exit(1);
    }
	log_message("MESSAGE QUEUE IS ALLOCATED");

	//criação dos semaforos responsaveis pela shared memory
	sem_unlink("shared");
	sem_shared = sem_open("shared", O_CREAT|O_EXCL, 0777,1);
	// Create processes
	create_proc();
	
	//pipe creation
	unlink(USER_PIPE);
	unlink(BACK_PIPE);
	create_pipes(USER_PIPE);
	create_pipes(BACK_PIPE);
}

void auth_request_manager(){
	log_message("PROCESS AUTHORIZATION_REQUEST_MANAGER CREATED");
	
	//create threads
	if (pthread_create(&sender_thread, NULL, sender_function, NULL) != 0)
	{
		log_message("CANNOT CREATE SENDER_THREAD");
		free_shared();
		exit(1);
	}

	if (pthread_create(&receiver_thread, NULL, receiver_function, NULL) != 0)
	{
		log_message("CANNOT CREATE RECEIVER_THREAD");
		free_shared();
		exit(1);
	}

	if(pthread_join(sender_thread, NULL)!= 0){
		log_message("CANNOT JOIN SENDER_THREAD");
		free_shared();
		exit(1);
	}

	if(pthread_join(receiver_thread, NULL)!= 0){
		log_message("CANNOT JOIN RECEIVER_THREAD");
		free_shared();
		exit(1);
	}
}

void monitor_engine(){
	log_message("PROCESS MONITOR_ENGINE CREATED");
} 

void create_proc(){
	auth_request_manager_pid = fork();
	if(auth_request_manager_pid == 0){
		auth_request_manager();
		return;
	}
	else if(auth_request_manager_pid < 0){
		log_message("AUTH_REQUEST_MANAGER FORK FAILED");
		free_shared();
		exit(1);
	}
	monitor_engine_pid = fork();
	if(monitor_engine_pid == 0){
		monitor_engine();
		return;
	}
	else if(monitor_engine_pid < 0){
		log_message("MONITOR_ENGINE FORK FAILED");
		free_shared();
		exit(1);
	}
	log_message("WAITING FOR CHILD PROCESSES TO FINISH");
    waitpid(auth_request_manager_pid, NULL, 0);
    waitpid(monitor_engine_pid, NULL, 0);

}

void *sender_function(void *arg){
	(void)arg;
	log_message("THREAD SENDER CREATED");

	return NULL;
}

void *receiver_function(void *arg){
	(void)arg;
	log_message("THREAD RECEIVER CREATED");
	if ((fd_read= open(USER_PIPE, O_RDWR)) < 0){
		log_message("ERROR OPENING PIPE FOR READING!");
		exit(1);
	
	}
	log_message("PIPE FOR READING IS OPEN!");

	while(1){
		fd_set read_set;
		FD_ZERO(&read_set);

		FD_SET(fd_read, &read_set);
	
		if(select(fd_read+1,&read_set,NULL,NULL,NULL)>0){
			if(FD_ISSET(fd_read,&read_set)){
				char buf[MAX_STRING_SIZE];
				int n=0;
				n=read(fd_read, buf, MAX_STRING_SIZE);

				buf[n]='\0';

				int cont=0;
				char *part1, *part2, *part3;
				part1 = strtok(buf, "#");
				if (part1 != NULL) {
					cont++;
					part2 = strtok(NULL, "#");
				}
				if (part2 != NULL) {
					cont++;
					part3 = strtok(NULL, "#");
				}
					
				if(cont==1){ //verificação dos dados e encaminhar para a respetiva função
					if(verificaS(part1)==2 && verificaS(part2)==2){
						addUser(&users,atoi(part1),atoi(part2));
						log_message("MOBILE USER ADDED TO SHARED MEMORY SUCCESSEFULLY.");
					}else	log_message("MOBILE USER SENT WRONG PARAMETERS.");
				}else if(cont==2){
					if(verificaS(part1)==2 && verificaS(part2)==1 && verificaS(part3)==2){
						auth_mobile(atoi(part1),part2,atoi(part3));
						log_message("MOBILE USER ADDED REQUEST SUCCESSEFULLY.");
					}else	log_message("MOBILE USER SENT WRONG PARAMETERS.");
				}else{
					log_message("MOBILE USER SENT WRONG PARAMETERS.");
				}
				
			}
		}
	}		
	return NULL;
}

bool validate_config(char* filename) {
    FILE *f = fopen(filename, "r");

    if (f == NULL) {
        log_message("UNABLE TO OPEN CONFIG FILE");
        return false;
    }
    int aux;

	//Read MAX_MOBILE_USER
	if (fscanf(f, "%d", &aux) < 1 || aux < 0) {
        snprintf(log_msg, sizeof(log_msg), "Error reading %s: MAX_MOBILE_USER %d must be >= 0", filename, aux);
        log_message(log_msg);
        fclose(f);
        return false;
    }
    config.max_mobile_user = aux;

    //Read QUEUE_POS
    if (fscanf(f, "%d", &aux) < 1 || aux < 0) {
        snprintf(log_msg, sizeof(log_msg), "Error reading %s: QUEUE_POS %d must be >= 0", filename, aux);
        log_message(log_msg);
        fclose(f);
        return false;
    }
    config.queue_slot_number = aux;

    //Read AUTH_SERVERS_MAX
    if (fscanf(f, "%d", &aux) < 1 || aux < 1) {
        snprintf(log_msg, sizeof(log_msg), "Error reading %s: AUTH_SERVERS_MAX %d must be >= 1", filename, aux);
        log_message(log_msg);
        fclose(f);
        return false;
    }
    config.max_auth_servers = aux;

    //Read AUTH_PROC_TIME
    if (fscanf(f, "%d", &aux) < 1 || aux < 0) {
        snprintf(log_msg, sizeof(log_msg), "Error reading %s: AUTH_PROC_TIME %d must be >= 0", filename, aux);
        log_message(log_msg);
        fclose(f);
        return false;
    }
    config.auth_proc_time = aux;

    //Read MAX_VIDEO_WAIT
    if (fscanf(f, "%d", &aux) < 1 || aux < 1) {
        snprintf(log_msg, sizeof(log_msg), "Error reading %s: MAX_VIDEO_WAIT %d must be >= 1", filename, aux);
        log_message(log_msg);
        fclose(f);
        return false;
    }
    config.max_video_wait = aux;

    //Read MAX_OTHERS_WAIT
    if (fscanf(f, "%d", &aux) < 1 || aux < 1) {
        snprintf(log_msg, sizeof(log_msg), "Error reading %s: MAX_OTHERS_WAIT %d must be >= 1", filename, aux);
        log_message(log_msg);
        fclose(f);
        return false;
    }
    config.max_others_wait = aux;

    fclose(f);
    return true;
}





void auth_mobile(int id, char type[MAX_STRING_SIZE], int amount){
	//fazer função que reparte os pedidos pelas respetivas filas
}