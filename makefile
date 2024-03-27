CC = gcc
LIBS = -lm
FLAGS = -Wall -pthread -g
PROGS = 5g_auth_platform mobile_user backoffice_user
HDRS = shared_mem.h system_manager.h log.h

all: $(PROGS)

clean:
	rm -f *.o *~ $(PROGS)

5g_auth_platform: system_manager.o log.o 
	$(CC) system_manager.o log.o $(FLAGS) $(LIBS) -o 5g_auth_platform

mobile_user: mobile_user.o log.o
	$(CC) mobile_user.o log.o $(FLAGS) $(LIBS) -o mobile_user

backoffice_user: backoffice_user.o log.o
	$(CC) backoffice_user.o log.o $(FLAGS) $(LIBS) -o backoffice_user

%.o: %.c $(HDRS)
	$(CC) -c $(FLAGS) $< -o $@
