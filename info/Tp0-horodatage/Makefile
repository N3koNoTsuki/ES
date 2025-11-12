# Makefile for info mode user
# Date : 12/09/2023
# Author : Pascal Aygalinc <Pascal.Aygalinc@univ-smb.fr>

ARCH = arm 
CROSS_COMPILE = arm-none-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc
INSTALL = install
CFLAGS =  -g -Wall
LDFLAGS = -g -Wall  

PROGS     = info

ifeq ($(prefix),)
    prefix	 = /home/geii/DE1-Soc/rootfs
endif

INSTDIR   = $(prefix)/home/root/
INSTMODE  = 0775
INSTOWNER = geii
INSTGROUP = geii

SRCS	= info.c  
OBJS	= info.o 

all:	$(PROGS)

$(PROGS): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(PROGS) *.o *~ core

install: $(PROGS)
	$(INSTALL) $(PROGS) -m $(INSTMODE) -t $(INSTDIR) 
#	$(INSTALL) -m $(INSTMODE) #-o $(INSTOWNER) -g $(INSTGROUP) $(PROGS) $(INSTDIR)

uninstall:
	rm -f $(INSTDIR)$(PROGS)
 
depend:
	makedepend -Y -- $(CFLAGS) -- $(SRCS) 2>/dev/null

# The following is used to automatically generate dependencies.
# DO NOT DELETE
info.o: info.h



