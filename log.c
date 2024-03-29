#include "log.h" 
#include "shared_mem.h"
#include <time.h>

// Function to log messages
int log_message(char* message) {
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);//ir buscar o tempo antes do lock para assim haver uma representação mais real da hora verdadeira em que a ação ocorreu
																//caso contrário, o tempo seria influenciado, cado o mutex estiver bloqueado, pela espera do mesmo

    // Adquire o mutex para escrita segura no arquivo de log
    pthread_mutex_lock(&log_mutex);

    FILE* log_file = fopen(FILENAME, "a");
    if (!log_file) {
        perror("Failed to open log file");
        pthread_mutex_unlock(&log_mutex);//liberar o mutex antes de sair para evitar deadlocks
		return 1;
        exit(EXIT_FAILURE);
    }

    // Escreve a mensagem de log com o timestamp capturado anteriormente
    fprintf(log_file, "%s - %s\n", buffer, message);
    // Também imprime na saída padrão
    printf("%s - %s\n", buffer, message);

    fclose(log_file);
    pthread_mutex_unlock(&log_mutex);
	return 0;
}