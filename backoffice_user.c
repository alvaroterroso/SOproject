//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

//TODO
//CRIAR RECEIVE DAS ESTATISTICAS DA MESSAGE QUEUE 
//CRIAR SOLICITAÇÃO DE ESTATISTICAS VIA PIPE PARA AUTHO MANAGER
//SEM FORKS() !!!!!!!!! - SÓ UM PROCESSO

#include "backoffice_user.h"
int fd_write;
int main(int argc, char **argv){
	log_message("BACKOFFICE USER IS RUNNING");

	if ((fd_write = open(BACK_PIPE, O_WRONLY)) < 0){
		log_message("ERROR OPENING PIPE FOR READING!");
		exit(1);
	}

	log_message("PIPE FOR READING IS OPEN!");
	
	//while para aceitar as coisas 
	while(1){
		scanf("%s", input);
		if(strcmp(input, data) == 0){
			printf("Requesting statistics...\n");
			write(fd_write, input, sizeof(input));
		}else if(strcmp(input, reset) == 0){
			printf("Reseting...\n");
			write(fd_write, input, sizeof(input));
		}else{
			printf("Command not accepeted! Usage: <1#data_stats> or <1#reset>\n");
		}
	}
}
