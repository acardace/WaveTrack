CC = gcc # C compiler
CFLAGS = -fPIC -O3 # C flags
LDFLAGS = -shared -Wl,-soname,libwavetrack.so.1 # linking flags
RM = rm -rf  # rm command
SOREALNAME = libwavetrack.so
SONAME = ${SOREALNAME}.1
TARGET_LIB = ${SONAME}.0.1 # target lib
DESTDIR = bin
LIBPATH = lib

SRCS = wavetrack.c # source files
OBJS = $(SRCS:.c=.o)

.PHONY: all clean install

all: ${TARGET_LIB}

$(TARGET_LIB): $(OBJS)
	 $(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c):%.c
	 $(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c)

clean:
	 -${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c) ${DESTDIR}

install:
	 @-mkdir -p ${DESTDIR}/usr/${LIBPATH}
	 @-install -Dm755 ${TARGET_LIB} ${DESTDIR}/usr/${LIBPATH}
	 @-ln -s ${TARGET_LIB} ${SONAME}
	 @-ln -s ${TARGET_LIB} ${SOREALNAME}
	 @-mv ${SONAME} ${SOREALNAME} ${DESTDIR}/usr/${LIBPATH}
