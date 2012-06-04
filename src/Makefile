CC = cc
CFLAGS = -std=c99 -Wall -O2 -pedantic -Wextra -D_GNU_SOURCE
CFLAGS += -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes \
	  -Wmissing-prototypes
CC_COMPILE = $(CC) $(CFLAGS) -c

OBJFILES := $(patsubst %.c,%.o,$(wildcard *.c))

PREFIX=/usr/local

all: wm

wm : $(OBJFILES)
	$(CC) -o wm $(OBJFILES) -lncurses -pthread

%.o: %.c
	$(CC_COMPILE) -o $@ $<

clean:
	@echo cleaning directory
	@rm -f *.o wm

install: all
	@echo installing wm to ${PREFIX}/bin
	@mkdir -p ${PREFIX}/bin
	@cp -f wm ${PREFIX}/bin
	@chmod 755 ${PREFIX}/bin/wm

version.h:
	@echo "#ifndef _VERSION_H_" >  version.h
	@echo "#define _VERSION_H_" >> version.h
	@echo -n "#define VERSION \"" >> version.h
	@git show -s --pretty=format:"dfm commit %h (%ai)\"%n" >> version.h
	@echo "#endif" >> version.h


