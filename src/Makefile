include config.mk

SRC = dawm.h main.c atoms.c bar.c client.c colors.c cursors.c ewmh.c keys.c \
      monitor.c rules.c settings.c sysinfo.c utils.c wm.c xutils.c layouts.c \
      x11.c
HEADERS = dawm.h
OBJ = ${SRC:.c=.o}

all: options version.h dawm

options:
	@echo dawm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o: ${HEADERS}
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: version.h

dawm: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning directory
	@rm -f *.o dawm

version.h:
	@echo "#ifndef _VERSION_H_" >  version.h
	@echo "#define _VERSION_H_" >> version.h
	@echo -n "#define VERSION \"" >> version.h
	@git show -s --pretty=format:"commit %h (%ai)\"%n" >> version.h
	@echo "#endif" >> version.h

test:
	@xinit ./dawm-test -- :1

.PHONY: clean
