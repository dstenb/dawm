# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# Xinerama
XINERAMALIBS = -L${X11LIB} -lXinerama
XINERAMAFLAGS = -DXINERAMA

# Xft
XFTINC = /usr/include/freetype2
XFTLIBS = -lXft -lfreetype
XFTFLAGS = -DXFT

# includes and libs
INCS = -I. -I/usr/include -I${X11INC} -I${XFTINC}
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 ${XINERAMALIBS} ${XFTLIBS} -lconfig

# flags
CPPFLAGS = ${XINERAMAFLAGS} ${XFTFLAGS}
CFLAGS = -std=c99 -pedantic -Wall -Wextra -D_GNU_SOURCE -Os \
	 -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes \
	 -Wmissing-prototypes ${INCS} ${CPPFLAGS}
LDFLAGS = -s ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC = cc
