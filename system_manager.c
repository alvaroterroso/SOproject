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
pthread_mutex_t mut_video = PTHREAD_MUTEX_INITIALIZER;   // Definição real
pthread_mutex_t mut_other = PTHREAD_MUTEX_INITIALIZER;   // Definição real
pthread_mutex_t mut_monitor = PTHREAD_MUTEX_INITIALIZER;   // Definição real
//pthread_mutex_t mut_cond = PTHREAD_MUTEX_INITIALIZER;   // Definição real
//pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

bool full = 0;
int adicional = 0;

//####################################################################################
//Criação dos semaforos necessários  e por dar inicio ao programa
//####################################################################################
int main(int argc, char **argv){
	sem_initializer();

	log_message("5G_AUTH_PLATFORM SIMULATOR STARTING");

	flag=0;
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

	strcpy(filename, argv[1]);
	if(!validate_config(filename)) exit(0);

	init_prog();

	if(getpid() == system_manager_pid){
		log_message("SIMULATOR WAITING FOR LAST TASKS TO FINISH");
		free_shared();
	}
	if(getpid() == system_manager_pid){
		log_message("5G_AUTH_PLATFORM SIMULATOR CLOSING\n");//last message
		sem_destroy(log_mutex);
	}
	return 0;
}
//Função que inicia os semaforos

void sem_initializer(){
	sem_unlink("shared");
	sem_unlink("counter");
	sem_unlink("read_count");
	sem_unlink("plafond");
	sem_unlink("mutex");
	sem_unlink("statics");
	sem_unlink("monitor");
	sem_unlink("mq_monitor");
	sem_unlink("back");
	sem_unlink("flag");
	sem_unlink("go");
	sem_unlink("run");
	sem_shared = sem_open("shared", O_CREAT|O_EXCL, 0777,1);
	if (sem_shared == SEM_FAILED) {
		log_message("ERROR OPENING SEM_SHARED");
    	exit(1);
	}
	sem_userscount = sem_open("counter",O_CREAT|O_EXCL, 0777,1);
	if (sem_userscount == SEM_FAILED) {
		log_message("ERROR OPENING SEM_USERSCOUNT");
    	exit(1);
	}
	sem_read_count = sem_open("read_count",O_CREAT|O_EXCL, 0777,1);
	if (sem_read_count == SEM_FAILED) {
		log_message("ERROR OPENING SEM_READ_COUNT");
    	exit(1);
	}
	sem_plafond = sem_open("plafond",O_CREAT|O_EXCL, 0777,1);
	if (sem_plafond == SEM_FAILED) {
		log_message("ERROR OPENING SEM_PLAFOND");
    	exit(1);
	}
	log_mutex= sem_open("mutex", O_CREAT|O_EXCL, 0777,1);
	if (log_mutex == SEM_FAILED) {
		log_message("ERROR OPENING SEM_LOG");
    	exit(1);
	}
	sem_statics = sem_open("statics",O_CREAT|O_EXCL, 0777,1);
	if (sem_statics == SEM_FAILED) {
		log_message("ERROR OPENING SEM_STATICS");
    	exit(1);
	}
	sem_monitor = sem_open("monitor",O_CREAT|O_EXCL, 0777,0);
	if (sem_monitor == SEM_FAILED) {
		log_message("ERROR OPENING SEM_MONITOR");
    	exit(1);
	}
	sem_flag = sem_open("flag",O_CREAT|O_EXCL, 0777,1);
	if (sem_flag == SEM_FAILED) {
		log_message("ERROR OPENING SEM_FLAG");
    	exit(1);
	}
	sem_go = sem_open("go",O_CREAT|O_EXCL, 0777,0);
	if (sem_go == SEM_FAILED) {
		log_message("ERROR OPENING SEM_GO");
    	exit(1);
	}
	sem_run = sem_open("run",O_CREAT|O_EXCL, 0777,1);
	if (sem_run == SEM_FAILED) {
		log_message("ERROR OPENING SEM_RUN");
    	exit(1);
	}
	sem_times = sem_open("times",O_CREAT|O_EXCL, 0777,1);
	if (sem_run == SEM_FAILED) {
		log_message("ERROR OPENING SEM_TIMES");
    	exit(1);
	}
}


//------------Função de verificação dos parametros-------------
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

//------------------Função que lida com o uso do ^C----------------------
void signal_handler(){
	free_shared();
	kill(0, SIGTERM); // kills all processes
	exit(0);
}

//-----------------Função que destroi semaforos, mutex, shared memory e message queue-----------------------
void free_shared(){
	sem_wait(sem_run);
	shared->run = 0;
	sem_post(sem_run);
	printf("OLA\n");
	//pthread_cancel(receiver_thread);
	pthread_join(receiver_thread,NULL);
	//pthread_cancel(sender_thread);
	pthread_join(sender_thread,NULL);
	printf("OLA1\n");
	close(fd_read_back);
	close(fd_read_user);
	printf("OLA2\n");
	unlink(BACK_PIPE);
	unlink(USER_PIPE);
	printf("OLA3\n");
	for(int i = 0; i < config.max_auth_servers; i ++){
		close(pipes[i][0]);
		close(pipes[i][1]);
	}
	printf("OLA4\n");
	//pthread_cancel(mobile_thread);
	pthread_join(mobile_thread,NULL);
	//pthread_cancel(back_thread);
	pthread_join(back_thread,NULL);
	printf("OLA5\n");
	destroyQueue(&q_video);
	destroyQueue(&q_other);
	sem_destroy(sem_shared);
	sem_destroy(sem_userscount);
	sem_destroy(sem_read_count);
	sem_destroy(sem_plafond);
	sem_destroy(sem_statics);
	sem_destroy(sem_monitor);
	sem_destroy(sem_flag);
	sem_destroy(sem_go);
	sem_destroy(sem_run);
	printf("OLA7\n");
	pthread_mutex_destroy(&mut_video);
	pthread_mutex_destroy(&mut_other);
	sem_unlink("statics");
	sem_unlink("shared");
	sem_unlink("counter");
	sem_unlink("read_count");
	sem_unlink("plafond");
	sem_unlink("video");
	sem_unlink("other");
	sem_unlink("monitor");
	sem_unlink("flag");
	sem_unlink("run");
	printf("OLA8\n");
	fcntl(fd_read_back,F_GETFL);
	close(fd_read_back);
	fcntl(fd_read_user,F_GETFL);
	close(fd_read_user);
	unlink(USER_PIPE);
	unlink(BACK_PIPE);
	printf("OLA9\n");
	shmdt(&shm_id);
	shmctl(shm_id, IPC_RMID, NULL);
	printf("OLA10\n");
	remove(MSQ_FILE);
	msgctl(mqid, IPC_RMID, 0); //DONE

}

//#########################################################################################
//Cria shared memory,  message queue e os processos Auth Requeste Manager e Monitor Engine
//#########################################################################################
void init_prog() {

	int shm_size = sizeof(shm) + sizeof(users_) * config.max_mobile_user + sizeof(int) * (config.max_auth_servers +1) + sizeof(stats_struct) + sizeof(int) *2 +sizeof(bool);

    if ((shm_id = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | IPC_EXCL | 0700)) < 0) {
        log_message("ERROR IN SHMGET");
        exit(1);
    }

    shared = (shm *)shmat(shm_id, NULL, 0);
    if (shared == (void *)-1) {
        log_message("ERROR IN SHMAT");
        exit(1);
    }
    log_message("SHARED MEMORY IS ALLOCATED");

	shared->user_array = (users_ *)((char *)shared + sizeof(shm));
	shared->read_count_shared = (int *)((char *)shared + sizeof(shm) + sizeof(users_) * config.max_auth_servers);

	//Inicialização do array de users(nao é preciso semaforo aqui porque mais ninguem tem acesso a isto)

    for (int i = 0; i < config.max_mobile_user; i++) {
        shared->user_array[i].id = -1;  
        shared->user_array[i].plafond = -1;  
        shared->user_array[i].plafond_ini = -1;  
		shared->user_array[i].times = 0;
    }

    // Inicializar array read_count_shared diretamente
    for (int i = 0; i < config.max_auth_servers + 1; i++) {
        shared->read_count_shared[i] = 0;
		log_message("AUTHORIZATION ENGINE %d READY");
    }

	shared->run = true;


	signal(SIGINT, signal_handler);//HANDLE CTRL-C

	// Inicializar struct_stats na message queue
	shared->stats.total_music = 0;
	shared->stats.total_video = 0;
	shared->stats.total_social = 0;
	shared->stats.music_req = 0;
	shared->stats.video_req = 0;
	shared->stats.social_req = 0;

    create_msq();
    create_proc();
}

//-------------------Cria em específico a message queue--------------------
void create_msq(){
  	if((mqid = msgget(IPC_PRIVATE, IPC_CREAT|0777)) == -1){
		log_message("ERROR CREATING MSG QUEUE");
		free_shared();
		exit(1);
  	}
	log_message("MESSAGE QUEUE IS ALLOCATED");
	FILE *fp = fopen(MSQ_FILE, "w");
	if (fp == NULL) {
		printf("ERROR OPENING FILE -> MSG_QUEUE_ID\n");
		exit(1);
	}
	fprintf(fp, "%d", mqid);
	fclose(fp);
}

//###########################################################
//Função que cria os Auth Request Manager e Monitor Engine
//###########################################################
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

//####################################################################################
//Função que cria os pipes, as threads SENDER e RECEIVER e os AUTHORIZATION ENGINES
//####################################################################################
void auth_request_manager(){
	log_message("PROCESS AUTHORIZATION_REQUEST_MANAGER CREATED");
	//create pipes
	create_unnamed_pipes();
	create_pipes(USER_PIPE);
	create_pipes(BACK_PIPE);
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

	create_autho_engines();

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

//------------------Cria em especifico os nammed pipes--------------------
void create_pipes(char * named){
	unlink(named);
	if ((mkfifo(named, O_CREAT | O_EXCL | 0700)<0) && (errno != EEXIST)){
    	log_message("CANNOT CREATE NAMED PIPE -> EXITING\n");
    	exit(1);
  	}
	snprintf(log_msg, sizeof(log_msg), "%s CREATED", named);
	log_message(log_msg);
}

//------------------Cria em especifico os unnammed pipess-------------------
void create_unnamed_pipes(){
    pipes = malloc((config.max_auth_servers + 1)* sizeof(int*));
    if (pipes == NULL) {
        perror("Falha na alocação de memória para pipes");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < config.max_auth_servers + 1; i++) {
        pipes[i] = malloc(2 * sizeof(int)); // Cada sub-array contém dois inteiros
        if (pipes[i] == NULL) {
            log_message("Falha na alocação de memória para sub-array de pipes");
			free_shared();
			exit(1);
        }

        if (pipe(pipes[i]) == -1) { // Criando o pipe
            perror("Falha ao criar pipe");
            exit(EXIT_FAILURE);
        }
    }
}

//##########################################################################################
//Função responsável por ler os Mobile Users e adicionar na fila VIDEO QUEUE ou OTHER QUEUE
//##########################################################################################
void *receiver_function(void *arg){
    (void)arg;
    log_message("THREAD RECEIVER CREATED");

    if ((fd_read_user= open(USER_PIPE, O_RDWR)) < 0){ //opening user for reading
        log_message("ERROR OPENING USER_PIPE FOR READING!");
        free_shared();
        exit(1);
    }
    log_message("USER_PIPE FOR READING IS OPEN!");

    if ((fd_read_back= open(BACK_PIPE,  O_RDWR)) < 0){//opening user for reading
        log_message("ERROR OPENING BACK_PIPE FOR READING!");
        free_shared();
        exit(1);
    }
    log_message("BACK_PIPE FOR READING IS OPEN!");
    
    int cont=0;
    char good_msg[MAX_STRING_SIZE] = {0};
    char buf[MAX_STRING_SIZE];
	sem_wait(sem_run);
    while(shared->run){
		sem_post(sem_run);
        fd_set read_set;
        FD_ZERO(&read_set);

        FD_SET(fd_read_user, &read_set);
        FD_SET(fd_read_back, &read_set);
        int nfds = (fd_read_user > fd_read_back ? fd_read_user : fd_read_back) + 1;
        if(select(nfds,&read_set,NULL,NULL,NULL)>0){
            if (FD_ISSET(fd_read_user, &read_set)) {
                int n = read(fd_read_user, buf, MAX_STRING_SIZE); //here
                //printf("Bits lidos -> %d\n", n);
                if (n > 0) {
                    buf[n] = '\0'; // Ensure the buffer is null-terminated
                    char *segment = strtok(buf, ";");  // Get the first token
                    while (segment != NULL) {  // Continue while there are tokens
                        printf("\n\nRECEIVED: %s [%d]\n\n", segment, ++cont);

                        // If you need to preserve the original message for further use after modification:
                        strcpy(good_msg, segment);  // Copy the current segment to good_msg
                        process_message_from_pipe(good_msg);  // Process the message

                        segment = strtok(NULL, ";");  // Get the next token
                    }
                }
            }else   log_message("\nERROR READING MESSAGE FROM NAMMED PIPE.\n");
            }
		if(FD_ISSET(fd_read_back,&read_set))
		{
			//mandar para others_queue
			char buf[MAX_STRING_SIZE];
			int n=read(fd_read_back, buf, MAX_STRING_SIZE);
			//printf("%s\n", buf);
			if(n>0){
				add_queue(&q_other,buf, mut_other);
				sem_post(sem_go);
				log_message("BACKOFFICE_USER REQUEST ADDED TO OTHERS QUEUE\n");
			}else log_message("\nERROR READING MESSAGE FROM NAMMED PIPE.\n");
		}
		sem_wait(sem_run);
    }
	sem_post(sem_run);
    return NULL;
}


void process_message_from_pipe(char * msg){

    char *part1, *part2, *part3;
    char copia[MAX_STRING_SIZE];
    strcpy(copia, msg);
	part1 = strtok(msg, "#");
	if (part1 != NULL) {
		part2 = strtok(NULL, "#");
	}
	if (part2 != NULL) {
		part3 = strtok(NULL, "#");
	}

    if ((verificaS(part1)==2) && (verificaS(part2)==1) && (verificaS(part3)==2)) { // Três partes: ID#TYPE#AMOUNT

        if (strcmp("VIDEO", part2) == 0) {
            if(countUsers(q_video,mut_video) < config.queue_slot_number){//ver se chegou ao limite na fila
                check_full(q_video,mut_video);
                add_queue(&q_video, copia, mut_video);
				sem_post(sem_go);

                log_message("MESSAGE ADDED TO VIDEO QUEUE.");
            }else{
				full = true; adicional = 1;
                log_message("VIDEO QUEUE IS FULL, DISCARDING...");
            }
        } else {
            if(countUsers(q_other,mut_other) < config.queue_slot_number){//ver se chegou ao limite na fila
                check_full(q_other,mut_other);
                add_queue(&q_other, copia, mut_other);
				sem_post(sem_go);
                log_message("MESSAGE ADDED TO OTHERS QUEUE.");
            }else{
				full = true; adicional = 1;
                log_message("OTHER QUEUE IS FULL, DISCARDING...");
            }
        }
    } else if ((verificaS(part1)== 2) && (verificaS(part2)== 2) && (part3==NULL)) { // Duas partes: ID#AMOUNT
        if(countUsers(q_other,mut_other) < config.queue_slot_number){//ver se chegou ao limite na fila
            check_full(q_other,mut_other);
            add_queue(&q_other, copia, mut_other);
			sem_post(sem_go);
            log_message("MESSAGE ADDED TO OTHERS QUEUE.(LOGIN)\n");
        }else{
			full = true; adicional = 1;
            log_message("OTHER QUEUE IS FULL, DISCARDING...");
        }
    } else {
        log_message("MOBILE USER SENT WRONG PARAMETERS.");
    }
}


//################################################################################################################
//Função que le os dados das filas, priorizando a fila de VIDEO e reencaminha pelos Unnamed Pipes o conteúdo lido
//################################################################################################################

void *sender_function(void *arg) {
    (void)arg;
    log_message("THREAD SENDER CREATED");

    while (1) {
		sem_wait(sem_go);

        if (!is_empty(q_video, mut_video, "VIDEO")) {
            while (!is_empty(q_video, mut_video,"VIDEO")) {
                process_queue_item(&q_video, mut_video, 1);
            }
        }

        // Processa a fila de "other" a cada ciclo da fila de vídeo
        if (!is_empty(q_other, mut_other, "OTHER")) {
            process_queue_item(&q_other, mut_other, 0);
        }

    }
    return NULL;
}

void check_full(queue *head, pthread_mutex_t sem){
	if (full){
		adicional = 1;
	}else if(countUsers(head, sem) <= config.queue_slot_number/2){
		full = false;
		adicional = 0;
	}
}

void process_queue_item(queue **q, pthread_mutex_t mut,int type) {
    bool found = false;
	sem_wait(sem_read_count);
    for (int i = 0; i < config.max_auth_servers + adicional; i++) {
        if (shared->read_count_shared[i] == 0) {
            shared->read_count_shared[i] = 1;
			sem_post(sem_read_count);
            *q = write_unnamed(*q, mut, i, type);
            found = true;
            break;
        }
    }
    if (!found) {
        sem_post(sem_read_count);
        log_message("No available auth engines, waiting...");
    }
}

//---------------Função que escreve nos Unnamed Pipes-------------
queue * write_unnamed(queue *q_some, pthread_mutex_t mut, int i,int type){

	char msg[MAX_STRING_SIZE];
	char *temp = rem_queue(&q_some, mut,type);

	if (temp) {
		strncpy(msg, temp, MAX_STRING_SIZE);
		msg[MAX_STRING_SIZE - 1] = '\0'; // Garantir terminação nula
	}else{
		log_message("REQUEST HAS EXCEEDED TIME LIMIT");
		return q_some;
	}

	char *part1, *part2, *part3;
	part1 = strtok(temp, "#");
	if (part1 != NULL) {
		part2 = strtok(NULL, "#");
	}
	if (part2 != NULL) {
		part3 = strtok(NULL, "#");
	}
	ssize_t num_written = write(pipes[i][1], msg, sizeof(msg));
	if (num_written == -1) {
		log_message("ERROR WRITING ON UNNAMED PIPE.");
		free_shared();
		exit(1);
	}else if(part3!=NULL){
		snprintf(log_msg, sizeof(log_msg),"SENDER: %s AUTHORIZATION REGUEST (ID = %s) SENT FOR PROCESSING ON AUTHORIZATION_ENGINE %d\n", part2, part1, i);
        log_message(log_msg);
	}else if(verificaS(part2)==2){
		snprintf(log_msg, sizeof(log_msg),"SENDER: LOGIN AUTHORIZATION REGUEST (ID = %s) SENT FOR PROCESSING ON AUTHORIZATION_ENGINE %d\n", part1, i);
        log_message(log_msg);
	}else{
		snprintf(log_msg, sizeof(log_msg),"SENDER: BACKOFFICE STATICS REGUEST (ID = %s) SENT FOR PROCESSING ON AUTHORIZATION_ENGINE %d\n", part1, i);
        log_message(log_msg);
	}
	return q_some;
}

//###################################################
//Cria em específico os Authorization Engines	    
//###################################################
void create_autho_engines(){                                           
	log_message("AUTHORIZATION_ENGINES PROCESSES BEING CREATED");
	autho_engines_pid = (pid_t*) malloc(config.max_auth_servers * sizeof(pid_t));
	for(int i = 0; i < config.max_auth_servers + 1; i++){
		autho_engines_pid[i] = fork();
		if(autho_engines_pid[i] == 0){
			read_from_unnamed(i);
			exit(0);
		}else if(autho_engines_pid[i] == -1){
			log_message("Error creating auth engines proccess.");
		}
	}
	while(wait(NULL)>0);
	return;
}

//-----------------Lê os unnamed pipes através do Authorization Engine-----------
void read_from_unnamed(int i){
	char message[MAX_STRING_SIZE];
	close(pipes[i][1]);
	sem_wait(sem_run);
	while(shared->run){
		sem_post(sem_run);
		int n;
		if((n = read(pipes[i][0],message, MAX_STRING_SIZE)) > 0){
			message[n]='\0';

			char *part1, *part2, *part3;
    		char copia[MAX_STRING_SIZE];
			strcpy(copia, message);

			part1 = strtok(copia, "#");
			if (part1 != NULL) {
				part2 = strtok(NULL, "#");
			}
			if (part2 != NULL) {
				part3 = strtok(NULL, "#");
			}
			if(part3 == NULL){}

			manage_auth(message);	

			if(part3 == NULL){
				if(*part1 ==1){
					sprintf(log_msg,"AUTHORIZATION ENGINE %d : BACK OFFICE REQUEST (ID = %s) PROCESSING COMPLETED\n",i,part1);
					log_message(log_msg);
				}else{
					sprintf(log_msg,"AUTHORIZATION ENGINE %d : LOGIN REQUEST (ID = %s) PROCESSING COMPLETED\n",i,part1);
					log_message(log_msg);	
				}
			}else{
				sprintf(log_msg,"AUTHORIZATION ENGINE %d : %s AUTHORIZATION REQUEST (ID = %s) PROCESSING COMPLETED\n",i,part2,part1);
				log_message(log_msg);
			}

			sleep(config.auth_proc_time);//durante o sleep o auth engine deve ser visto como indisponivel certo?????????????????????

			sem_wait(sem_read_count);

			shared->read_count_shared[i] = 0;	

			sem_post(sem_read_count);

		}else	log_message("EROR READING FROM UNNAMED PIPE.");
	}
	sem_post(sem_run);
	return;
}


void add_stats(char *msg) {
    //printf("MESSAGE TO GO TO ADD_STATS: %s\n", msg);
	char token[MAX_STRING_SIZE];
    char service[MAX_STRING_SIZE];  // Initialize the buffer to zero
    int plaf = 0;
	strcpy(token, msg);
	char *part1, *part2, *part3;
	part1 = strtok(token, "#");
	if (part1 != NULL) {
		part2 = strtok(NULL, "#");
	}
	if (part2 != NULL) {
		part3 = strtok(NULL, "#");
	}

	strcpy(service, part2);
	plaf = atoi(part3);


    //printf("STATS HAVE BEEN UPDATED\n");
    // Compare the service string and update the corresponding statistics
	sem_wait(sem_statics);
    if (strcmp(service, "VIDEO") == 0) {
        shared->stats.total_video += plaf;
        shared->stats.video_req += 1;
    } else if (strcmp(service, "MUSIC") == 0) {
        shared->stats.total_music += plaf;
        shared->stats.music_req += 1;
    } else if (strcmp(service, "SOCIAL") == 0) {
        shared->stats.total_social += plaf;
        shared->stats.social_req += 1;
    }
	sem_post(sem_statics);
	//printf("\nSTATS HAVE BEEN UPDATED\n");
}

//---------------Esta função é responsável por tratar os dados lidos dos Unnamed Pipes----------------

void manage_auth(char *buf){
    char copia[MAX_STRING_SIZE];
    strcpy(copia,buf);

    char *part1, *part2, *part3;
    part1 = strtok(buf, "#");
    if (part1 != NULL) {
        part2 = strtok(NULL, "#");
    }
    if (part2 != NULL) {
        part3 = strtok(NULL, "#");
    }
    if(verificaS(part1)==2 && verificaS(part2)==2 && part3==NULL){ //login
        sem_wait(sem_userscount);

        if(shared->mobile_users<config.max_mobile_user){
            shared->mobile_users++;
            addUser(atoi(part1),atoi(part2));
            log_message("MOBILE USER ADDED TO SHARED MEMORY SUCCESSEFULLY.");

        }else{
            //enviar mq ao mobile user para dizer que está cheio
			sem_wait(sem_flag);
			flag=atoi(part1);
			sem_post(sem_flag);
            log_message("MOBILE USER LIST IS FULL, NOT GOING TO LOGIN.");
        }
        sem_post(sem_userscount);
    }else if(verificaS(part1)==2 && verificaS(part2)==1 && verificaS(part3)==2){//pedido de autorização

        int user_index = searchUser(atoi(part1));
        if(user_index == -1){
            log_message("MOBILE USER NOT FOUND.");
        }else{
            sem_wait(sem_plafond);
			if(shared->user_array[user_index].plafond < atoi(part3)){
				log_message("MOBILE USER [%d] DOESNT HAVE ENOUGH PLAFOND TO CONTINUE...");
				sem_post(sem_plafond);
				sem_wait(sem_flag);
				flag=atoi(part1);
				sem_post(sem_flag);
			}else{
				shared->user_array[user_index].plafond = shared->user_array[user_index].plafond - atoi(part3);
				sem_post(sem_plafond);
				log_message("MOBILE USER ADDED REQUEST SUCCESSEFULLY.");
				sem_post(sem_monitor);
				if(count_char_occurrences(copia,'#') == 2){
					add_stats(copia);
				}
			}
        
        }
    }else if(atoi(part1) == 1) { // mensagem do BACK_OFFICE

        if(strcmp(part2, "reset") == 0){

            sem_wait(sem_statics);
            shared->stats.total_video = 0;
            shared->stats.total_social = 0;
            shared->stats.total_music = 0;
            shared->stats.music_req = 0;
            shared->stats.video_req = 0;
            shared->stats.video_req = 0;
            sem_post(sem_statics);
            log_message("STATS RESETED!\n");
        }else{
            sem_wait(sem_statics);
			char buff[1024];
			sprintf(buff, "Service\tTotal Data\tAuth Reqs\nVIDEO\t%d\t%d\nMUSIC\t%d\t%d\nSOCIAL\t%d\t%d\n",
			shared->stats.total_video, shared->stats.video_req, shared->stats.total_music, shared->stats.music_req,
			shared->stats.total_social, shared->stats.social_req);
			sem_post(sem_statics);
			plafond_msg monitor;
			monitor.id=(long)1;
			strcpy(monitor.msg, buff);
			monitor.msg[sizeof(monitor.msg) - 1] = '\0';
			int mqid_= get_msg_id();
			msgsnd(mqid_,&monitor,sizeof(monitor)-sizeof(long),0);
            log_message("STATS SENDED TO BACK_OFFICE\n");
        }
    }
}

//#####################################################################
//Funções responsáveis por manipular as FILAS DE VIDEO e OTHER
//#####################################################################

//-----------------Adiciona um user à sua Fila-----------------
void add_queue(queue **head, const char *message, pthread_mutex_t sem) {
    // Criar um novo nó
    queue *newNode = malloc(sizeof(queue));
    if (newNode == NULL) {
        fprintf(stderr, "Falha na alocação de memória\n");
        return;
    }
    strncpy(newNode->message, message, MAX_STRING_SIZE);
    newNode->message[MAX_STRING_SIZE - 1] = '\0';  // Garantir terminação nula
    newNode->next = NULL;
	newNode->time = current_time_millis();

    pthread_mutex_lock(&sem);
    if (*head == NULL) {
        // Se a fila está vazia, o novo nó se torna o cabeçalho
        *head = newNode;
        pthread_mutex_unlock(&sem);
        return;
    } else {
        // Caso contrário, encontrar o último nó
        queue *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
        pthread_mutex_unlock(&sem);
        return;
    }
}

//--------------Remove o user da sua Fila---------------------------------
char *rem_queue(queue **head, pthread_mutex_t sem,int type) {
    pthread_mutex_lock(&sem);
    if (*head == NULL) {
        pthread_mutex_unlock(&sem);
        return NULL;
    }

    queue *temp = *head;  // Point to the head of the queue
    char *message = malloc(strlen(temp->message) + 1);  // +1 for null terminator
    if (message == NULL) {
        pthread_mutex_unlock(&sem);

        return NULL;  // Handle allocation failure
    }

    // Copy the message from the queue node to the newly allocated memory
    strcpy(message, temp->message);
    *head = temp->next;  // Point the head to the next element

	long long now = current_time_millis();
    long long elapsed = now - temp->time;

    free(temp);
    pthread_mutex_unlock(&sem);

	if(type == 1){
		if(elapsed > config.max_video_wait)
			return NULL;
	}else{
		if(elapsed > config.max_others_wait)
			return NULL;
	}

    return message;  // Return the duplicated message
}

void destroyQueue(queue **head) {
    if (head == NULL || *head == NULL) return;  // Check if the queue pointer or head is NULL

    queue *current = *head;  // Start with the head of the queue
    queue *next;

    while (current != NULL) {  // Traverse each node until the end
        next = current->next;  // Store the next node
        free(current);         // Free the current node
        current = next;        // Move to the next node
    }

    *head = NULL;  // After all nodes are freed, reset the head to NULL
}


//----------------Verifica se a Fila está Vazia----------------------
int is_empty(queue *head, pthread_mutex_t sem,char tipo[MAX_STRING_SIZE]){  //1 se está vazia, 0 tem elementos
    pthread_mutex_lock(&sem);
    if(head==NULL){
        pthread_mutex_unlock(&sem);
        //print_queue(head, sem,tipo); 
        return 1;
    }else{
        pthread_mutex_unlock(&sem);
        //print_queue(head, sem, tipo); 
        return 0;
    }
}

void print_queue(queue *head, pthread_mutex_t sem, char tipo[MAX_STRING_SIZE]) {
    pthread_mutex_lock(&sem); // Garante acesso exclusivo à fila
    queue *current = head;
    
    printf("Current Queue [%s]:\n", tipo);
    if (current == NULL) {
        printf("The queue is empty.\n");
    }
    while (current != NULL) {
        printf("Message: %s\n", current->message);
        current = current->next;
    }
    printf("Fim do print da fila\n");
    pthread_mutex_unlock(&sem); // Libera o acesso à fila
}

int countUsers(queue *head, pthread_mutex_t sem) {
    int count = 0;  // Inicializa contador
    pthread_mutex_lock(&sem);
    queue *current = head;  // Começa pelo primeiro nó da fila

    // Percorre a lista até o final
    while (current != NULL) {
        count++;  // Incrementa o contador para cada nó encontrado
        current = current->next;  // Move para o próximo nó
    }
    pthread_mutex_unlock(&sem);
    return count;  // Retorna o número total de nós
}

//MONITOR ENGINE

void monitor_engine(){
    log_message("PROCESS MONITOR_ENGINE CREATED");
    mq = get_msg_id();
    if (pthread_create(&mobile_thread, NULL, plafond_function, NULL) != 0)
    {
        log_message("CANNOT CREATE SENDER_THREAD");
        free_shared();
        exit(0);
    }

    if (pthread_create(&back_thread, NULL, statics_function, NULL) != 0)
    {
        log_message("CANNOT CREATE BACK_THREAD");
        free_shared();
        exit(0);
    }

    if(pthread_join(mobile_thread, NULL)!= 0){
        log_message("CANNOT JOIN MOBILE THREAD");
        free_shared();
        exit(1);
    }

    if(pthread_join(back_thread, NULL)!= 0){
        log_message("CANNOT JOIN BACK THREAD");
        free_shared();
        exit(1);
    }

} 

void *plafond_function(){
	sem_wait(sem_run);
    while(shared->run){
		sem_post(sem_run);
        sem_wait(sem_monitor);
		sem_wait(sem_flag);
		if(flag!=0){//se a flag por diferente de 0, é porque houve um user que nao conseguiu dar login, e a flag tem o seu id para eu saber que type usar na mensagem
			plafond_msg monitor;
			monitor.id = (long)flag;
			flag=0;
			sem_post(sem_flag);
			strcpy(monitor.msg, MOB_FULL);
			monitor.msg[sizeof(monitor.msg) - 1] = '\0';
			msgsnd(mq,&monitor,sizeof(monitor)-sizeof(long),0);
		}
		else{
			sem_post(sem_flag);
			log_message("------GOING TO CHECK EVERYONE PLAFOND------\n");
			for(int i =0; i<config.max_mobile_user; i++){
				if((shared->user_array[i].id > 1)){
					plafond_msg monitor;
					sem_wait(sem_plafond);
					float plafond_gasto = (1 - (shared->user_array[i].plafond/shared->user_array[i].plafond_ini));
					sem_post(sem_plafond);
					snprintf(log_msg, sizeof(log_msg), "PLAFOND GASTO USER[%d]: %.3f\n",(int) shared->user_array[i].id, plafond_gasto);
					log_message(log_msg);

					if(plafond_gasto == 1 ){
						
						sem_wait(sem_times);
						if(shared->user_array[i].times <= 2){
							monitor.id= (long)shared->user_array->id;
							strcpy(monitor.msg, PLA_100);
							monitor.msg[sizeof(monitor.msg) - 1] = '\0';
							msgsnd(mq,&monitor,sizeof(monitor)-sizeof(long),0);
							shared->user_array[i].times += 1;
						}
						sem_post(sem_times);

					}else if(plafond_gasto > 0.9){

						sem_wait(sem_times);
						if(shared->user_array[i].times <= 1){
							monitor.id= (long)shared->user_array->id;
							strcpy(monitor.msg, PLA_90);
							monitor.msg[sizeof(monitor.msg) - 1] = '\0';
							msgsnd(mq,&monitor,sizeof(monitor)-sizeof(long),0);
							shared->user_array[i].times += 1;
						}
						sem_post(sem_times);

					}else if(plafond_gasto > 0.8){
						sem_wait(sem_times);
						if(shared->user_array[i].times == 0){
							monitor.id= (long)shared->user_array->id;
							strcpy(monitor.msg, PLA_80);
							monitor.msg[sizeof(monitor.msg) - 1] = '\0';
							msgsnd(mq,&monitor,sizeof(monitor)-sizeof(long),0);
							shared->user_array[i].times += 1;
						}
						sem_post(sem_times);

					}
				}
			}
		}
		sem_wait(sem_run);
    }
	sem_post(sem_run);
    return NULL;
}

void *statics_function(){
	sem_wait(sem_run);
	while(shared->run){
		sem_post(sem_run);
		sleep(30);
		sem_wait(sem_statics);
        char buff[1024];
        sprintf(buff, "---PERIODIC STATS---\nService\tTotal Data\tAuth Reqs\nVIDEO\t%d\t%d\nMUSIC\t%d\t%d\nSOCIAL\t%d\t%d\n",
        shared->stats.total_video, shared->stats.video_req, shared->stats.total_music, shared->stats.music_req,
        shared->stats.total_social, shared->stats.social_req);
        sem_post(sem_statics);
        plafond_msg monitor;
        monitor.id=(long)1;
        strcpy(monitor.msg, buff);
        monitor.msg[sizeof(monitor.msg) - 1] = '\0';
        msgsnd(mq,&monitor,sizeof(monitor)-sizeof(long),0);
		log_message("PERIODIC STATS HAVE BEEN SENT");
		sem_wait(sem_run);
	}
	sem_post(sem_run);
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

long long current_time_millis() {
    struct timeval time;
    if (gettimeofday(&time, NULL) == 0) {
        return (long long)time.tv_sec * 1000 + (long long)time.tv_usec / 1000;
    } else {
        perror("Failed to get time");
        return -1;
    }
}