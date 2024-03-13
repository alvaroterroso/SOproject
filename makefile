CC = gcc
LIBS = -lm
FLAGS = -Wall -pthread -g
PROGS = system_manager mobile_user backoffice_user

all: $(PROGS)

clean:
	rm -f *.o *~ $(PROGS)

system_manager: system_manager.o 
	$(CC) $(FLAGS) $^ $(LIBS) -o $@

mobile_user: mobile_user.o
	$(CC) $(FLAGS) $^ $(LIBS) -o $@

backoffice_user: backoffice_user.o
	$(CC) $(FLAGS) $^ $(LIBS) -o $@

%.o: %.c %.h shared_mem.h
	$(CC) $(FLAGS) -c $< -o $@



