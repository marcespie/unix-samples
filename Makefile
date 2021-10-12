ALLPROGS = fork1 fork2 fork3 fork3bad fork3bad2 fork4 fork5 \
	pipe1 pipe2 pipe3 pipe4 pipe5 pipe5bad

PIPE3OBJS = myfuncs.o pipe3.o
PIPE4OBJS = myfuncs.o pipe4.o
PIPE5OBJS = myfuncs.o pipe5.o
PIPE5BADOBJS = myfuncs.o pipe5bad.o

all: ${ALLPROGS}


pipe3: ${PIPE3OBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE3OBJS}

pipe4: ${PIPE4OBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE4OBJS}

pipe5: ${PIPE5OBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE5OBJS}

pipe5bad: ${PIPE5BADOBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE5BADOBJS}

clean:
	-rm -f ${ALLPROGS} ${PIPE3OBJS} ${PIPE4OBJS}

.PHONY: all
