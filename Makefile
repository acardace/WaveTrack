CC = gcc # C compiler
CFLAGS = -fPIC -O3 # C flags
LDFLAGS = -shared  # linking flags
RM = rm -f  # rm command
TARGET_LIB = libwavetrack.so # target lib

SRCS = wavetrack.c # source files
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: ${TARGET_LIB}

$(TARGET_LIB): $(OBJS)
	 $(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c):%.c
	 $(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c)

clean:
	 -${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)
