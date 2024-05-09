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
pthread_mutex_t mut_cond = PTHREAD_MUTEX_INITIALIZER;   // Definição real
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

//####################################################################################
//Criação dos semaforos necessários  e por dar inicio ao programa
//####################################################################################
int main(int argc, char **argv){
	sem_unlink("shared");
	sem_unlink("counter");
	sem_unlink("read_count");
	sem_unlink("plafond");
	//sem_unlink("control");
	sem_unlink("mutex");
	sem_unlink("login");
	sem_unlink("statics");
	sem_unlink("monitor");
	sem_unlink("mq_monitor");
	sem_shared = sem_open("shared", O_CREAT|O_EXCL, 0777,1);
	sem_userscount = sem_open("counter",O_CREAT|O_EXCL, 0777,1);
	sem_read_count = sem_open("read_count",O_CREAT|O_EXCL, 0777,1);
	sem_plafond = sem_open("plafond",O_CREAT|O_EXCL, 0777,1);
	//sem_controlar = sem_open("control", O_CREAT|O_EXCL, 0777,0);
	log_mutex= sem_open("mutex", O_CREAT|O_EXCL, 0777,1);
	sem_login1st = sem_open("login",O_CREAT|O_EXCL, 0777,0);
	sem_statics = sem_open("statics",O_CREAT|O_EXCL, 0777,1);
	sem_monitor = sem_open("monitor",O_CREAT|O_EXCL, 0777,0);
	mq_monitor = sem_open("mq_monitor",O_CREAT|O_EXCL, 0777,1);


	log_message("5G_AUTH_PLATFORM SIMULATOR STARTING");


	run = 1;
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

	//int pipes[config.max_auth_servers][2];// [0,0,0,0,0,0,0,0] -> [1,0,0,0,0,0,0] -> [1,1,0,0,0,0,0] -> [0,1,0,0,0,0,0,0] -> [1,1,0,0,0,0,0,0]
	//ESTA LINHA NAO É PRECISA PORQUE DECLAREI NO.H E NO CREATE UNNAMED EU JA FAÇO ISSO DIREITO
	init_prog();

	signal(SIGINT, signal_handler);//HANDLE CTRL-C

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
	run=0;
	sem_destroy(sem_shared);
	sem_destroy(sem_userscount);
	sem_destroy(sem_read_count);
	sem_destroy(sem_plafond);
	//sem_destroy(sem_controlar);
	sem_destroy(sem_login1st);
	sem_destroy(sem_statics);
	sem_destroy(sem_monitor);
	pthread_mutex_destroy(&mut_video);
	pthread_mutex_destroy(&mut_other);
	pthread_mutex_destroy(&mut_cond);
	sem_unlink("statics");
	sem_unlink("shared");
	sem_unlink("counter");
	sem_unlink("read_count");
	sem_unlink("plafond");
	//sem_unlink("control");
	sem_unlink("video");
	sem_unlink("other");
	sem_unlink("login");
	sem_unlink("monitor");
	shmdt(&shm_id);
	shmctl(shm_id, IPC_RMID, NULL);
	msgctl(mqid, IPC_RMID, 0); //DONE
	unlink(BACK_PIPE);
	unlink(USER_PIPE);
	for(int i = 0; i < config.max_auth_servers; i ++){
		close(pipes[i][0]);
		close(pipes[i][1]);
	}
}

//#########################################################################################
//Cria shared memory,  message queue e os processos Auth Requeste Manager e Monitor Engine
//#########################################################################################
void init_prog() {
    // Tamanho da estrutura shm mais o espaço necessário para o array read_count_shared
    int shm_size = sizeof(shm) + sizeof(users_) * config.max_mobile_user + sizeof(int) * config.max_auth_servers + sizeof(stats_struct);

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
    }

    // Inicializar array read_count_shared diretamente
    for (int i = 0; i < config.max_auth_servers; i++) {
        shared->read_count_shared[i] = 0;
    }

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
    pipes = malloc(config.max_auth_servers * sizeof(int*));
    if (pipes == NULL) {
        perror("Falha na alocação de memória para pipes");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < config.max_auth_servers; i++) {
        pipes[i] = malloc(2 * sizeof(int)); // Cada sub-array contém dois inteiros
        if (pipes[i] == NULL) {
            perror("Falha na alocação de memória para sub-array de pipes");
            exit(EXIT_FAILURE);
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

	if ((fd_read_user= open(USER_PIPE, O_RDONLY | O_NONBLOCK)) < 0){ //opening user for reading
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
	int cont=0;

	while(1){
		fd_set read_set;
		FD_ZERO(&read_set);

		FD_SET(fd_read_user, &read_set);
		FD_SET(fd_read_back, &read_set);
		int nfds = (fd_read_user > fd_read_back ? fd_read_user : fd_read_back) + 1;
		if(select(nfds,&read_set,NULL,NULL,NULL)>0){
			if (FD_ISSET(fd_read_user, &read_set)) {
                char buf[MAX_STRING_SIZE];
                int n = read(fd_read_user, buf, MAX_STRING_SIZE);
                if (n > 0) {
					if(buf[0] == '\0')
						break;
					printf("LI DO USER\n");
					cont ++;
                    buf[n] = '\0';
                    char copy[MAX_STRING_SIZE];
                    strncpy(copy, buf, sizeof(copy) - 1);
                    copy[sizeof(copy) - 1] = '\0';

                    char *part1, *part2, *part3;
					part1 = strtok(buf, "#");
					if (part1 != NULL) {
						part2 = strtok(NULL, "#");
					}
					if (part2 != NULL) {
						part3 = strtok(NULL, "#");
					}

					printf("\n\nRECEBI: %s [%d]\n\n", copy, cont);

                    if ((verificaS(part1)==2) && (verificaS(part2)==1) && (verificaS(part3)==2)) { // Três partes: ID#TYPE#AMOUNT

                        if (strcmp("VIDEO", part2) == 0) {
							if(countUsers(q_video,mut_video) <= config.queue_slot_number){//ver se chegou ao limite na fila
								pthread_mutex_lock(&mut_cond);
								add_queue(&q_video, copy, mut_video);
								pthread_cond_signal(&cond);
								pthread_mutex_unlock(&mut_cond);

								log_message("MESSAGE ADDED TO VIDEO QUEUE.");
							}else{
								log_message("VIDEO QUEUE IS FULL, DISCARDING...");
							}
                        } else {
							if(countUsers(q_other,mut_other) <= config.queue_slot_number){//ver se chegou ao limite na fila
								pthread_mutex_lock(&mut_cond);
								add_queue(&q_other, copy, mut_other);
								pthread_cond_signal(&cond);
								pthread_mutex_unlock(&mut_cond);
								log_message("MESSAGE ADDED TO OTHERS QUEUE.");
							}else{
								log_message("OTHER QUEUE IS FULL, DISCARDING...");
							}
                        }
                    } else if ((verificaS(part1)== 2) && (verificaS(part2)== 2) && (part3==NULL)) { // Duas partes: ID#AMOUNT
						if(countUsers(q_other,mut_other) <= config.queue_slot_number){//ver se chegou ao limite na fila
							pthread_mutex_lock(&mut_cond);
							add_queue(&q_other, copy, mut_other);
							pthread_cond_signal(&cond);
							pthread_mutex_unlock(&mut_cond);

							log_message("MESSAGE ADDED TO OTHERS QUEUE.(LOGIN)\n");
						}else{
							log_message("OTHER QUEUE IS FULL, DISCARDING...");
						}
                    } else {
                        log_message("MOBILE USER SENT WRONG PARAMETERS.");
                    }
                }else	log_message("\nERROR READING MESSAGE FROM NAMMED PIPE.\n");
            }
			if(FD_ISSET(fd_read_back,&read_set))
			{
				//mandar para others_queue
				char buf[MAX_STRING_SIZE];
				int n=read(fd_read_back, buf, MAX_STRING_SIZE);
				//printf("%s\n", buf);
				if(n>0){
					pthread_mutex_lock(&mut_cond);
					add_queue(&q_other,buf, mut_other);
					pthread_cond_signal(&cond);
					pthread_mutex_unlock(&mut_cond);
					log_message("BACKOFFICE_USER REQUEST ADDED TO OTHERS QUEUE\n");
				}else log_message("\nERROR READING MESSAGE FROM NAMMED PIPE.\n");
			}
		}
	}
	return NULL;
}


//################################################################################################################
//Função que le os dados das filas, priorizando a fila de VIDEO e reencaminha pelos Unnamed Pipes o conteúdo lido
//################################################################################################################

void *sender_function(void *arg) {
    (void)arg;
    log_message("THREAD SENDER CREATED");

    while (run) {
		pthread_mutex_lock(&mut_cond);
		while(is_empty(q_video, mut_video, "VIDEO") && is_empty(q_other, mut_other, "OTHER")){
			pthread_cond_wait(&cond, &mut_cond);
		}

        // Processa a fila de vídeo
        if (!is_empty(q_video, mut_video, "VIDEO")) {
            while (!is_empty(q_video, mut_video,"VIDEO")) {
                process_queue_item(&q_video, mut_video);
            }
        }

        // Processa a fila de "other" a cada ciclo da fila de vídeo
        if (!is_empty(q_other, mut_other, "OTHER")) {
            process_queue_item(&q_other, mut_other);
        }
		pthread_mutex_unlock(&mut_cond);
    }
    return NULL;
}

void process_queue_item(queue **q, pthread_mutex_t mut) {
    bool found = false;
	sem_wait(sem_read_count);
	printf("ARRAY DE UNNAMED PIPES, VERIFICAR LIVRES: ");
    for (int i = 0; i < config.max_auth_servers; i++) {
		printf("%d ",shared->read_count_shared[i]);
        if (shared->read_count_shared[i] == 0) {
			printf("\n");
            shared->read_count_shared[i] = 1;
			sem_post(sem_read_count);
            *q = write_unnamed(*q, mut, i);
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
queue * write_unnamed(queue *q_some, pthread_mutex_t mut, int i){

	char msg[MAX_STRING_SIZE];
	char *temp = rem_queue(&q_some, mut);

	if (temp) {
		strncpy(msg, temp, MAX_STRING_SIZE);
		msg[MAX_STRING_SIZE - 1] = '\0'; // Garantir terminação nula
	}
	
	ssize_t num_written = write(pipes[i][1], msg, sizeof(msg));
	printf("ACABEI DE ESCREVER NO UNNAMED : %s\n", msg);

	if (num_written == -1) {
		log_message("ERROR WRITING ON UNNAMED PIPE.");
		run=0;
		free_shared();
		exit(1);
	}
	return q_some;
}

//###################################################
//Cria em específico os Authorization Engines	    
//###################################################
void create_autho_engines(){
	log_message("AUTHORIZATION_ENGINES PROCESSES BEING CREATED");
	autho_engines_pid = (pid_t*) malloc(config.max_auth_servers * sizeof(pid_t));
	for(int i = 0; i < config.max_auth_servers; i++){
		autho_engines_pid[i] = fork();
		if(autho_engines_pid[i] == 0){
			read_from_unnamed(i);
			exit(0);
		}
	}
}

//-----------------Lê os unnamed pipes através do Authorization Engine-----------
void read_from_unnamed(int i){
	char message[MAX_STRING_SIZE];
	close(pipes[i][1]);
	while(run){//ISTO É CONSIDERADO ESPERA ATIVA??????????????????????????????????????????????????
		int n;
		if((n = read(pipes[i][0],message, MAX_STRING_SIZE)) > 0){
			message[n]='\0';
			printf("LI DO UNNAMED PIPE : %s\n", message);
			if(count_char_occurrences(message,'#') == 2){
				add_stats(message);
			}
			sem_wait(sem_read_count);

			shared->read_count_shared[i] = 0;		//acabou de ler portanto pomos a 0 denovo
			printf("\n\nREPUS O INDICE [%d] DO ARRAY A 0 DENOVO\n",i);
			for (int i = 0; i < config.max_auth_servers; i++) {
				printf("%d ",shared->read_count_shared[i]);
			}
			printf("\n");
			sem_post(sem_read_count);
			manage_auth(message);
		}else	log_message("EROR READING FROM UNNAMED PIPE.");
	}
}

//---------------Esta função é responsável por tratar os dados lidos dos Unnamed Pipes----------------

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
	printf("\nSTATS HAVE BEEN UPDATED\n");
}


void manage_auth(char *buf){

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
			sem_post(sem_login1st);
			log_message("MOBILE USER ADDED TO SHARED MEMORY SUCCESSEFULLY.");
		}else{
			//################################################################################################################################################
			//temos de enviar algo de modo que o mobile user saiba que nao foi logado e portanto tem de terminar o seu processo(talvez variaveis de condição)
			//################################################################################################################################################
			log_message("MOBILE USER LIST IS FULL, NOT GOING TO LOGIN.");
		}
		sem_post(sem_userscount);
	}else if(verificaS(part1)==2 && verificaS(part2)==1 && verificaS(part3)==2){//pedido de autorização
		sem_wait(sem_login1st);
		int user_index = searchUser(atoi(part1));
		if(user_index == -1){
			log_message("MOBILE USER NOT FOUND.");
		}else{
			sem_wait(sem_plafond);
			shared->user_array[user_index].plafond = shared->user_array[user_index].plafond - atoi(part3);

			if(shared->user_array[user_index].plafond < 0)	shared->user_array[user_index].plafond = 0;
			
			float plafond_gasto= (1 - (shared->user_array[user_index].plafond/shared->user_array[user_index].plafond_ini));
			printf("\nPLAFOND GASTO: %.3f\n", plafond_gasto);
			sem_post(sem_plafond);
			log_message("MOBILE USER ADDED REQUEST SUCCESSEFULLY.");

			if(plafond_gasto == 1 ){

				sem_wait(mq_monitor);
				monitor.id= (long)shared->user_array->id;
				strcpy(monitor.msg, PLA_100);
				monitor.msg[sizeof(monitor.msg) - 1] = '\0';
				sem_post(mq_monitor);
				sem_post(sem_monitor);
				printf("\nLIBEREI O SEMAFORO PARA A MQ\n");

			}else if(plafond_gasto > 0.9){

				sem_wait(mq_monitor);
				monitor.id= (long)shared->user_array->id;
				strcpy(monitor.msg, PLA_90);
				monitor.msg[sizeof(monitor.msg) - 1] = '\0';
				sem_post(mq_monitor);
				sem_post(sem_monitor);
				printf("\nLIBEREI O SEMAFORO PARA A MQ\n");

			}else if(plafond_gasto > 0.8){

				sem_wait(mq_monitor);
				monitor.id= (long)shared->user_array->id;
				strcpy(monitor.msg, PLA_80);
				monitor.msg[sizeof(monitor.msg) - 1] = '\0';
				sem_post(mq_monitor);
				sem_post(sem_monitor);
				printf("\nLIBEREI O SEMAFORO PARA A MQ\n");

			}
			//FAZER PARTE DE VERIFICAR A PERCENTAGEM DE PLAFOND QUE TEM
		
		}
		sem_post(sem_login1st);
	}else if(strcmp(buf,reset) == 0){
		sem_wait(sem_statics);
		shared->stats.total_video = 0;
		shared->stats.total_social = 0;
		shared->stats.total_music = 0;
		shared->stats.music_req = 0;
		shared->stats.video_req = 0;
		shared->stats.video_req = 0;
		sem_post(sem_statics);
		log_message("STATS RESETED!\n");
	}else if(strcmp(buf, data) == 0){
		sem_wait(sem_statics);
		char buff[1024];
		sprintf(buff, "Service\tTotal Data\tAuth Reqs\nVIDEO\t%d\t%d\nMUSIC\t%d\t%d\nSOCIAL\t%d\t%d\n",
		shared->stats.total_video, shared->stats.video_req, shared->stats.total_music, shared->stats.music_req,
		shared->stats.total_social, shared->stats.social_req);
		sem_post(sem_statics);
		//SEND TO MESSAGE QUEUE
		sem_wait(mq_monitor);
		monitor.id = (long)1;
		strcpy(monitor.msg, buf);
		sem_post(mq_monitor);
		sem_post(sem_monitor);
		//printf("\nID DO BACKOFFICE QUE ENVIEI MQ: %ld\n", back_msg.id);
		log_message("STATS SENDED TO BACK_OFFICE\n");
	}else{
		//################################################################################################################################################
		//temos de enviar algo de modo que o mobile user saiba que nao foi logado e portanto tem de terminar o seu processo(talvez variaveis de condição)
		//################################################################################################################################################
		log_message("MOBILE USER SENT WRONG PARAMETERS.");
	}
	sleep(config.auth_proc_time);
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
char *rem_queue(queue **head, pthread_mutex_t sem) {
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

	free(temp);

    pthread_mutex_unlock(&sem);
    return message;  // Return the duplicated message
}


//----------------Verifica se a Fila está Vazia----------------------
int is_empty(queue *head, pthread_mutex_t sem,char tipo[MAX_STRING_SIZE]){	//1 se está vazia, 0 tem elementos
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


void monitor_engine(){
	log_message("PROCESS MONITOR_ENGINE CREATED");
	while(1){
		sem_wait(sem_monitor);
		printf("\n\nVOU ENVIAR UMA MQ\n\n");
		msgsnd(mqid,&monitor,sizeof(monitor)-sizeof(long),0);
	}
} 