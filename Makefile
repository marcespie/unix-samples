ALLPROGS = fork1 fork2 fork3 fork3bad fork3bad2 fork4 fork5 fork6 \
	pipe1 pipe1bis pipe2 pipe3 pipe4 pipe4bad server1

FORK6OBJS = myfuncs.o fork6.o
PIPE1OBJS = myfuncs.o pipe1.o
PIPE1BISOBJS = myfuncs.o pipe1bis.o
PIPE2OBJS = myfuncs.o pipe2.o
PIPE3OBJS = myfuncs.o pipe3.o
PIPE4OBJS = myfuncs.o pipe4.o
PIPE4BADOBJS = myfuncs.o pipe4bad.o
SERVER1OBJS = myfuncs.o server1.o

all: ${ALLPROGS}

pipe1: ${PIPE1OBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE1OBJS}

pipe1bis: ${PIPE1BISOBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE1BISOBJS}

pipe2: ${PIPE2OBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE2OBJS}

pipe3: ${PIPE3OBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE3OBJS}

pipe4: ${PIPE4OBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE4OBJS}

fork6: ${FORK6OBJS}
	${CC} -o $@ ${CFLAGS} ${FORK6OBJS}

pipe4bad: ${PIPE4BADOBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE4BADOBJS}

server1: ${SERVER1OBJS}
	${CC} -o $@ ${CFLAGS} ${SERVER1OBJS}
clean:
	-rm -f ${ALLPROGS} *.o

.PHONY: all clean
