//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

/*
TODO:
->CRIAR FILAS DE VIDEO E A OTHERS, E ENVIAR OS DADOS DO MOBILE USER PARA A FILA CORRESPONDENTE
->METODO PARA AVISAR SE O MOBILE USER  FOI OU NAO REGISTADO
VER SE ESTÁ CERTO:
->VER SE O MOBILE USER CONSEGUE DAR LOGIN 

*/

#include "system_manager.h"


int main(int argc, char **argv){
	log_message("5G_AUTH_PLATFORM SIMULATOR STARTING");
	sem_shared = sem_open("shared", O_CREAT|O_EXCL, 0777,1);
	sem_userscount = sem_open("counter",O_CREAT|O_EXCL, 0777,1);
	sem_read_count = sem_open("read_count",O_CREAT|O_EXCL, 0777,1);
	//ignore signal while inittilazing 

	/*
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
 	*/

	system_manager_pid = getpid();

	log_message("PROCESS SYSTEM_MANAGER CREATED");
	if (argc < 2) {
        fprintf(stderr, "Usage: %s <config-file-path> \n", argv[0]);
        return 1;
    }

	strcpy(filename, argv[1]);
	if(!validate_config(filename)) exit(0);

	int pipes[config.max_auth_servers][2];// [0,0,0,0,0,0,0,0] -> [1,0,0,0,0,0,0] -> [1,1,0,0,0,0,0] -> [0,1,0,0,0,0,0,0] -> [1,1,0,0,0,0,0,0]
	int read_count[config.max_auth_servers];
	create_unnamed_pipes(pipes);
	init_prog();

	signal(SIGINT, signal_handler);//HANDLE CTRL-C

	if(getpid() == system_manager_pid){
		log_message("SIMULATOR WAITING FOR LAST TASKS TO FINISH");
		free_shared();
	}
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
	if ((mkfifo(named, O_CREAT | O_EXCL | 0700)<0) && (errno != EEXIST)){
    	log_message("CANNOT CREATE NAMED PIPE -> EXITING\n");
    	exit(1);
  	}
	snprintf(log_msg, sizeof(log_msg), "%s CREATED", named);
	log_message(log_msg);
}

void free_shared(){
	printf("in\n");
	shmdt(&shm_id);
	shmctl(shm_id, IPC_RMID, NULL);
	msgctl(mqid, IPC_RMID, 0); //DONE
	unlink(BACK_PIPE);
	unlink(USER_PIPE);
}

void create_msq(){
  	if((mqid = msgget(IPC_PRIVATE, IPC_CREAT|0777)) == -1){
		log_message("ERROR CREATING MSG QUEUE");
		free_shared();
		exit(1);
  	}
	log_message("MESSAGE QUEUE IS ALLOCATED");
}

void init_prog(){
	int shm_size = sizeof(shm) + sizeof(users_)* config.max_mobile_user;
	if ((shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | IPC_EXCL | 0700)) < 1){
    	log_message("ERROR IN SHMGET");
    	exit(1);
  	}
	shared = (shm *)shmat(shm_id, NULL, 0);
	if(shared == (void*)-1){
		log_message("ERROR IN SHMAT");
		exit(1);
	}
	log_message("SHARED MEMORY IS ALLOCATED");

	//init count
	sem_wait(sem_read_count);
	for(int i = 0; i < config.max_auth_servers; i++){
		shared->read_count_shared[i] = 0;
	}
	sem_post(sem_read_count);

	create_pipes(USER_PIPE);
	create_pipes(BACK_PIPE);
	create_msq();
	// Create processes
	create_proc();

	//criação dos semaforos responsaveis pela shared memory
	sem_unlink("shared");
	sem_unlink("counter");
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

	if(pthread_join(receiver_thread, NULL)!= 0){
		log_message("CANNOT JOIN RECEIVER_THREAD");
		free_shared();
		exit(1);
	}

	if(pthread_join(sender_thread, NULL)!= 0){
		log_message("CANNOT JOIN SENDER_THREAD");
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

void create_unnamed_pipes(int pipes[][2]){
	for (int i = 0; i < config.max_auth_servers; i++) {
		if (pipe(pipes[i]) == -1) {
			log_message("CANNOT CREATE UNNAMED PIPE -> EXITING\n");
			exit(1);
		}
	}
}

void create_autho_engines(int pipes[][2]){
	log_message("AUTHORIZATION_ENGINES PROCESSES CREATED");
	autho_engines_pid = (pid_t*) malloc(config.max_auth_servers * sizeof(pid_t));
	for(int i = 0; i < config.max_auth_servers; i++){
		autho_engines_pid[i] = fork();
		if(autho_engines_pid[i] == 0){
			read_from_unnamed(pipes[i], i);
			exit(0);
		}
	}
}

void read_from_unnamed(int pipes[2], int i){
	close(pipes[1]);
	if(read(pipes[0], ) > 0){
		sem_wait(sem_read_count);
		shared->read_count_shared[i] = 1;
		sem_post(sem_read_count);

	}
	sem_wait(sem_read_count);
	shared->read_count_shared[i] = 0;
	sem_post(sem_read_count);
}

void *sender_function(void *arg){
	(void)arg;
	log_message("THREAD SENDER CREATED");
	printf("sender function created\n");
	return NULL;
}

void *receiver_function(void *arg){
	(void)arg;
	log_message("THREAD RECEIVER CREATED");

	if ((fd_read_user= open(USER_PIPE, O_RDONLY | O_NONBLOCK)) < 0){ //opening user for reading
		printf("%d\n", errno);
		log_message("ERROR OPENING USER_PIPE FOR READING!");
		free_shared();
		exit(1);
	}
	log_message("USER_PIPE FOR READING IS OPEN!");

	if ((fd_read_back= open(BACK_PIPE, O_RDONLY| O_NONBLOCK)) < 0){//opening user for reading
		log_message("ERROR OPENING BACK_PIPE FOR READING!");
		free_shared();
		exit(1);
	}
	log_message("BACK_PIPE FOR READING IS OPEN!");

	printf("VOU COMEÇAR A LER: A ENTRAR NO WHILE\n");
	while(1){
		fd_set read_set;
		FD_ZERO(&read_set);

		FD_SET(fd_read_user, &read_set);
		FD_SET(fd_read_back, &read_set);
		int nfds = (fd_read_user > fd_read_back ? fd_read_user : fd_read_back) + 1;
		if(select(nfds,&read_set,NULL,NULL,NULL)>0){
			if(FD_ISSET(fd_read_user,&read_set)){
				log_message("A LER O QUE FOI ENVIADO");
				char buf[MAX_STRING_SIZE];
				int n=0;
				n=read(fd_read_user, buf, MAX_STRING_SIZE);
				printf("%s\n", buf);

				buf[n]='\0';
				int cont=0;
				char *part1, *part2, *part3;
				part1 = strtok(buf, "#");
				if (part1 != NULL) {
					part2 = strtok(NULL, "#");
				}
				if (part2 != NULL) {
					part3 = strtok(NULL, "#");
				}
				printf("part1 -> %s\n part2-> %s\n",part1,part2);
				printf("print da funçãoi part1: %d\nprint da funçãoi part2: %d\ncount:%d\n",verificaS(part1),verificaS(part2),cont);
				if(verificaS(part1)==2 && verificaS(part2)==2 && part3==NULL){ //verificação dos dados e encaminhar para a respetiva função
					sem_wait(sem_userscount);
					if(shared->mobile_users<config.max_mobile_user){
						++shared->mobile_users;
						sem_post(sem_userscount);
						addUser(&shared->users,atoi(part1),atoi(part2));
						log_message("MOBILE USER ADDED TO SHARED MEMORY SUCCESSEFULLY.");
					}else{
						sem_post(sem_userscount);
						//temos de enviar algo de modo que o mobile user saiba que nao foi logado e portanto tem de terminar o seu processo
						log_message("MOBILE USER LIST IS FULL, NOT GOING TO LOGIN.");
					}
				}else if(verificaS(part1)==2 && verificaS(part2)==1 && verificaS(part3)==2){
					auth_mobile(atoi(part1),part2,atoi(part3));
					log_message("MOBILE USER ADDED REQUEST SUCCESSEFULLY.");
				}else{
					log_message("MOBILE USER SENT WRONG PARAMETERS.");
				}
				
			}
			if(FD_ISSET(fd_read_back,&read_set)){
				char buf[MAX_STRING_SIZE] = "\0";
				int n=read(fd_read_back, buf, MAX_STRING_SIZE);
				if(n > 0){
					printf("%s\n", buf);
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