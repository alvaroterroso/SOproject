CC = gcc
LIBS = -lm
FLAGS = -Wall -pthread -g
PROGS = 5g_auth_platform mobile_user backoffice_user
HDRS = shared_mem.h system_manager.h

all: $(PROGS)

clean:
	rm -f *.o *~ $(PROGS)

5g_auth_platform: system_manager.o 
	$(CC) $^ $(FLAGS) $(LIBS) -o $@

mobile_user: mobile_user.o
	$(CC) $^ $(FLAGS) $(LIBS) -o $@

backoffice_user: backoffice_user.o
	$(CC) $^ $(FLAGS) $(LIBS) -o $@

%.o: %.c $(HDRS)
	$(CC) $(FLAGS) -c $< -o $@
