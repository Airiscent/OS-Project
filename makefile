CC=gcc
CFLAGS=
RM = rm -f

all: a.out

a.out: OSProject.c
	$(CC) $(CFLAGS) -o a.out OSProject.c
clean:
	$(RM) a.out

