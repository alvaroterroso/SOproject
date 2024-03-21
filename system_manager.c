//TODO
//LER O FICHEIRO & validar os dados
//CRIAR MEMÓRIA PARTILHADA
//CRIAR PROCESSOS WORKER
//CRIAR PROCESSOS ALERT WATCHER
//CRIAR THREADS
#include "system_manager.h"

int main(int argc, char **argv){
	printf("System starting...\n");
	if (argc < 2) {
        fprintf(stderr, "Usage: %s <config-file-path>\n", argv[0]);
        return 1;
    }
	strcpy(filename, argv[1]);
	if(!validate_config(filename)) exit(0);
	printf("%s read\n", filename);
	init_prog();
	return 0;
}

void init_prog(){
	//size incomplete
	int shm_size = sizeof(config_struct) + sizeof(mobile_user_struct);
	if ((shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | IPC_EXCL | 0700)) < 1){
    	printf("ERROR IN SHMGET\n");
    	exit(1);
  	}
	if(shmat(shm_id, NULL, 0) == (void*)-1){
		printf("ERROR IN SHMAT\n");
		exit(0);
	}
	// Create threads
	if (pthread_create(&sender_thread, NULL, sender_function, NULL) != 0)
	{
		printf("CANNOT CREATE SENDER_THREAD\n");
		exit(1);
	}

	if (pthread_create(&receiver_thread, NULL, receiver_function, NULL) != 0)
	{
		printf("CANNOT CREATE RECEIVER_THREAD\n");
		exit(1);
	}

	if(pthread_join(sender_thread, NULL)!= 0){
		printf("CANNOT JOIN SENDER_THREAD\n");
		exit(1);
	}

	if(pthread_join(receiver_thread, NULL)!= 0){
		printf("CANNOT JOIN RECEIVER_THREAD\n");
		exit(1);
	}
}

void *sender_function(void *arg){
	(void)arg;
	printf("sender thread functional...\n");
	return NULL;
}

void *receiver_function(void *arg){
	(void)arg;
	printf("receiver thread functional...\n");
	return NULL;
}

bool validate_config(char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        perror("Unable to open the file");
        return false;
    }
    int aux;
    if (fscanf(f, "%d", &aux) < 1 || aux < 0) { // QUEUE_POS
        printf("Error reading %s: %d must be >= 0\n", filename, aux);
        fclose(f);
        return false;
    }
    config.queue_slot_number = aux;

    if (fscanf(f, "%d", &aux) < 1 || aux < 1) { // AUTH_SERVERS_MAX
        printf("Error reading %s: %d must be >= 1\n", filename, aux);
        fclose(f);
        return false;
    }
    config.max_auth_servers = aux;

    if (fscanf(f, "%d", &aux) < 1 || aux < 0) { // AUTH_PROC_TIME
        printf("Error reading %s: %d must be >= 0\n", filename, aux);
        fclose(f);
        return false;
    }
    config.auth_proc_time = aux;

    if (fscanf(f, "%d", &aux) < 1 || aux < 1) { // MAX_VIDEO_WAIT
        printf("Error reading %s: %d must be >= 1\n", filename, aux);
        fclose(f);
        return false;
    }
    config.max_video_wait = aux;

    if (fscanf(f, "%d", &aux) < 1 || aux < 1) { // MAX_OTHERS_WAIT
        printf("Error reading %s: %d must be >= 1\n", filename, aux);
        fclose(f);
        return false;
    }
    config.max_others_wait = aux;

    fclose(f);
    return true;
}
