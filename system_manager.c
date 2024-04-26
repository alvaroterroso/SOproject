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
	if ((mkfifo(named, O_CREAT|O_EXCL|0600)<0) && (errno != EEXIST)){
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
	int shm_size = sizeof(config_struct) + sizeof(mobile_user_struct) * config.max_mobile_user;
	if ((shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | IPC_EXCL | 0700)) < 1){
    	log_message("ERROR IN SHMGET");
    	exit(1);
  	}
	if(shmat(shm_id, NULL, 0) == (void*)-1){
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

	// Create processes
	create_proc();
	
	//pipe creation
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
