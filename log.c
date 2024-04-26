//Álvaro Terroso 2021213782
//Rui Oliveira 2022210616

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
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);//fetch current time for more accurate output
																//if not, the time would be affected by the mutex pause

	//Get the mutex to write to the log file
    pthread_mutex_lock(&log_mutex);

    FILE* log_file = fopen(FILENAME, "a");
    if (!log_file) {
        perror("Failed to open log file");
        pthread_mutex_unlock(&log_mutex);//free mutex to avoid deadlocks
		return 1;
        exit(EXIT_FAILURE);
    }

	//Writes message with timestamp captured on the top
    fprintf(log_file, "%s - %s\n", buffer, message);
    //Also prints on the console
    printf("%s - %s\n", buffer, message);

    fclose(log_file);
    pthread_mutex_unlock(&log_mutex);
	return 0;
}