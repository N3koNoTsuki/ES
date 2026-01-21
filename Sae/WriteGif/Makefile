ARCH = arm 
CROSS_COMPILE = arm-none-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc
INSTALL = install
CFLAGS =  -g -Wall 
LDFLAGS = -g -Wall  


PROGS     = WriteGif

ifeq ($(prefix),)
    prefix	 = /home/geii/DE1-Soc/rootfs
endif

INSTDIR   = $(prefix)/home/root/
INSTMODE  = 0775

SRCS	= WriteGif.c function.c
OBJS	= WriteGif.o function.o

all:	$(PROGS)

$(PROGS): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(PROGS) *.o *~ core

install: $(PROGS)
	$(INSTALL) $(PROGS) -m $(INSTMODE) -t $(INSTDIR)
	rm -f $(PROGS) *.o *~ core

uninstall:
	rm -f $(INSTDIR)$(PROGS)
 
depend:
	makedepend -Y -- $(CFLAGS) -- $(SRCS) 2>/dev/null

WriteGif.o: WriteGif.h 
function.o : WriteGif.h