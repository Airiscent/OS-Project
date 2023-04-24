CC=gcc
CFLAGS=
RM = rm -f

all: exe

exe: OSProject.c
	$(CC) $(CFLAGS) -o exe OSProject.c
clean:
	$(RM) exe

