EXECUTABLES = myshell attacker
CFLAGS = -g -Wall

all : myshell attacker
.PHONY : all

myshell : myshell.c
	gcc $(CFLAGS) -o myshell myshell.c

attacker : attacker.c
	gcc $(CFLAGS) -o attacker attacker.c

clean :
	rm $(EXECUTABLES) 
