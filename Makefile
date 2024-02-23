CC=gcc
CFLAGS=-Wall -g -std=gnu99 -pedantic
OBJECTFILES=daemon.o

all:	daemon

daemon:	$(OBJECTFILES)
	$(CC) $(CFLAGS) -o $@ $^ -lnetconf2 -lyang -lsysrepo

%.o:	%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTFILES) daemon

.PHONY: all clean
