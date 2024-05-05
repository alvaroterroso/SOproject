#Álvaro Terroso 2021213782
#Rui Oliveira 2022210616

CC = gcc
FLAGS = -Wall -pthread -g
PROGS = 5g_auth_platform mobile_user backoffice_user

all: $(PROGS)

clean:
	rm -f *.o *~ $(PROGS)

5g_auth_platform: system_manager.o log.o
	${CC} ${FLAGS} $^ -o $@

mobile_user: mobile_user.o log.o
	${CC} ${FLAGS} $^ -o $@

backoffice_user: backoffice_user.o log.o
	${CC} ${FLAGS} $^ -o $@

%.o: %.c
	${CC} ${FLAGS} -c $< -o $@

####################

system_manager.o: system_manager.c system_manager.h shared_mem.h log.h mobile_user.h backoffice_user.h
log.o: log.c log.h shared_mem.h
mobile_user.o: mobile_user.c mobile_user.h shared_mem.h log.h
backoffice_user.o: backoffice_user.c backoffice_user.h shared_mem.h log.h system_manager.h