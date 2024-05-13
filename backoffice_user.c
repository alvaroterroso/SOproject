//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

//TODO
//CRIAR RECEIVE DAS ESTATISTICAS DA MESSAGE QUEUE 
//CRIAR SOLICITAÇÃO DE ESTATISTICAS VIA PIPE PARA AUTHO MANAGER
//SEM FORKS() !!!!!!!!! - SÓ UM PROCESSO

#include "backoffice_user.h"
int fd_write;
int mq;
plafond_msg back_msg_rcv;
int main(int argc, char **argv){
    printf("BACKOFFICE USER IS RUNNING\n");

    if ((fd_write = open(BACK_PIPE, O_WRONLY)) < 0){
        printf("ERROR OPENING PIPE FOR READING!");
        exit(1);
    }

    printf("PIPE FOR WRITTING IS OPEN!\n");
    mq = get_msg_id();
    pid_t fork_ = fork();
    if(fork_==0){
        printf("in\n");
        ler_mq();
        exit(0);
    }
	signal(SIGINT, signal_handle);
    
    //while para aceitar as coisas
    while(1){
        scanf("%s", input);
        if(strcmp(input, data) == 0){
            printf("Requesting statistics...\n");
            write(fd_write, input, sizeof(input)); //write to pipe
            printf("%s\n", back_msg_rcv.msg);
        }else if(strcmp(input, reset) == 0){
            printf("Reseting...\n");
            write(fd_write, input, sizeof(input)); //write to pipe
        }else{
            printf("Command not accepeted! Usage: <1#data_stats> or <1#reset>\n");
        }
    }
}

void signal_handle(){
	printf("BACKOFFICE AS ENDED\n"),
	run = 0;
	close(fd_write);
	exit(0);
}

void ler_mq(){
    while(1){
        msgrcv(mq,&back_msg_rcv,sizeof(back_msg_rcv)-sizeof(long),(long)1,0);
        printf("MENSAGEM RECEBIDA:\n%s",back_msg_rcv.msg);
    }
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
