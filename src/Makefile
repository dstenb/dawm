CC = cc
CFLAGS = -std=c99 -Wall -O2 -pedantic -Wextra -D_GNU_SOURCE
CFLAGS += -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes \
	  -Wmissing-prototypes
CC_COMPILE = $(CC) $(CFLAGS) -c

OBJFILES := $(patsubst %.c,%.o,$(wildcard *.c))

PREFIX=/usr/local

all: clean version.h dawm

dawm : $(OBJFILES)
	$(CC) -o dawm $(OBJFILES) -lX11

%.o: %.c
	$(CC_COMPILE) -o $@ $<

clean:
	@echo cleaning directory
	@rm -f *.o dawm

install: all
	@echo installing dawm to ${PREFIX}/bin
	@mkdir -p ${PREFIX}/bin
	@cp -f dawm ${PREFIX}/bin
	@chmod 755 ${PREFIX}/bin/dawm

version.h:
	@echo "#ifndef _VERSION_H_" >  version.h
	@echo "#define _VERSION_H_" >> version.h
	@echo -n "#define VERSION \"" >> version.h
	@git show -s --pretty=format:"commit %h (%ai)\"%n" >> version.h
	@echo "#endif" >> version.h

test:
	@xinit ./dawm-test -- :1

.PHONY: clean
