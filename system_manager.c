//TODO
//LER O FICHEIRO & validar os dados - done
//CRIAR MEMÓRIA PARTILHADA - done
//CRIAR PROCESSOS WORKER - done
//CRIAR PROCESSOS ALERT WATCHER - done
//CRIAR THREADS - done

#include "system_manager.h"
//pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv){

	log_message("5G_AUTH_PLATFORM SIMULATOR STARTING");
	if (argc < 2) {
        fprintf(stderr, "Usage: %s <config-file-path> \n", argv[0]);
        return 1;
    }
	mobile_user_count = 0;
	strcpy(filename, argv[1]);
	if(!validate_config(filename)) exit(0);
	printf("%s read\n", filename);
	init_prog();
	free_shared(shm_id);
	log_message("5G_AUTH_PLATFORM SIMULATOR CLOSING\n");
	return 0;
}

void free_shared(int shm_id){
	shmdt(&shm_id);
	shmctl(shm_id, IPC_RMID, NULL);
	log_message("SIMULATOR WAITING FOR LAST TASKS TO FINISH") ;
}

void init_prog(){
	//size incomplete
	int shm_size = sizeof(config_struct) + sizeof(mobile_user_struct);
	if ((shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | IPC_EXCL | 0700)) < 1){
    	log_message("ERROR IN SHMGET");
    	exit(1);
  	}
	if(shmat(shm_id, NULL, 0) == (void*)-1){
		log_message("ERROR IN SHMAT");
		exit(0);
	}
	log_message("SHARED MEMORY IS ALLOCATED");
	// Create threads
	if (pthread_create(&sender_thread, NULL, sender_function, NULL) != 0)
	{
		log_message("CANNOT CREATE SENDER_THREAD");
		free_shared(shm_id);
		exit(1);
	}

	if (pthread_create(&receiver_thread, NULL, receiver_function, NULL) != 0)
	{
		log_message("CANNOT CREATE RECEIVER_THREAD");
		free_shared(shm_id);
		exit(1);
	}

	if(pthread_join(sender_thread, NULL)!= 0){
		log_message("CANNOT JOIN SENDER_THREAD");
		free_shared(shm_id);
		exit(1);
	}

	if(pthread_join(receiver_thread, NULL)!= 0){
		log_message("CANNOT JOIN RECEIVER_THREAD");
		free_shared(shm_id);
		exit(1);
	}
	create_proc();
}

void auth_request_manager(){
	log_message("PROCESS AUTHORIZATION_REQUEST_MANAGER CREATED");
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
		free_shared(shm_id);
		exit(1);
	}
	monitor_engine_pid = fork();
	if(monitor_engine_pid == 0){
		monitor_engine();
		return;
	}
	else if(monitor_engine_pid < 0){
		log_message("MONITOR_ENGINE FORK FAILED");
		free_shared(shm_id);
		exit(1);
	}

	// Esperar pela terminação dos processos filho para evitar processos zumbi
    waitpid(auth_request_manager_pid, NULL, 0);
    waitpid(monitor_engine_pid, NULL, 0);
    log_message("WAITING FOR CHILD PROCESSES TO FINISH");
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

bool validate_config(char *filename) {
    FILE *f = fopen(filename, "r");// Define a variável aqui, garantindo que seja grande o suficiente para a mensagem.

    if (f == NULL) {
        log_message("UNABLE TO OPEN CONFIG FILE");
        return false;
    }
    int aux;

    // Lendo QUEUE_POS
    if (fscanf(f, "%d", &aux) < 1 || aux < 0) {
        snprintf(log_msg, sizeof(log_msg), "Error reading %s: QUEUE_POS %d must be >= 0", filename, aux);
        log_message(log_msg);
        fclose(f);
        return false;
    }
    config.queue_slot_number = aux;

    // Lendo AUTH_SERVERS_MAX
    if (fscanf(f, "%d", &aux) < 1 || aux < 1) {
        snprintf(log_msg, sizeof(log_msg), "Error reading %s: AUTH_SERVERS_MAX %d must be >= 1", filename, aux);
        log_message(log_msg);
        fclose(f);
        return false;
    }
    config.max_auth_servers = aux;

    // Lendo AUTH_PROC_TIME
    if (fscanf(f, "%d", &aux) < 1 || aux < 0) {
        snprintf(log_msg, sizeof(log_msg), "Error reading %s: AUTH_PROC_TIME %d must be >= 0", filename, aux);
        log_message(log_msg);
        fclose(f);
        return false;
    }
    config.auth_proc_time = aux;

    // Lendo MAX_VIDEO_WAIT
    if (fscanf(f, "%d", &aux) < 1 || aux < 1) {
        snprintf(log_msg, sizeof(log_msg), "Error reading %s: MAX_VIDEO_WAIT %d must be >= 1", filename, aux);
        log_message(log_msg);
        fclose(f);
        return false;
    }
    config.max_video_wait = aux;

    // Lendo MAX_OTHERS_WAIT
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
