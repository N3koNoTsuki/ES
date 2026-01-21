CC = gcc
INSTALL = install
CFLAGS =  -g -Wall 
LDFLAGS = -g -Wall  
PROGS     = ReadFile
INSTDIR   = /home/nekonotsuki/Documents/Perso/C/BIN		# A modifier selon votre configuration
INSTMODE  = 0775
SRCS	= main.c 
OBJS	= main.o

all:	$(PROGS)

$(PROGS): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(PROGS)
	$(INSTALL) $(PROGS) -m $(INSTMODE) -t $(INSTDIR) 
	rm -f $(PROGS) *.o *~ core
uninstall:
	rm -f $(INSTDIR)$(PROGS)

main.o: main.h