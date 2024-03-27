#ifndef LOG_H
#define LOG_H

#include <pthread.h>

// Declaração do mutex para uso externo, se necessário.
extern pthread_mutex_t log_mutex;

// Protótipo da função log_message.
void log_message(const char* message);

#endif // LOG_H
