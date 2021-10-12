ALLPROGS = fork1 fork2 fork3 fork3bad fork3bad2 fork4 fork5 fork6 \
	pipe1 pipe2 pipe3 pipe4 pipe5 pipe5bad

FORK6OBJS = myfuncs.o fork6.o
PIPE1OBJS = myfuncs.o pipe1.o
PIPE3OBJS = myfuncs.o pipe3.o
PIPE4OBJS = myfuncs.o pipe4.o
PIPE5OBJS = myfuncs.o pipe5.o
PIPE5BADOBJS = myfuncs.o pipe5bad.o

all: ${ALLPROGS}

pipe1: ${PIPE1OBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE1OBJS}

pipe3: ${PIPE3OBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE3OBJS}

pipe4: ${PIPE4OBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE4OBJS}

pipe5: ${PIPE5OBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE5OBJS}

fork6: ${FORK6OBJS}
	${CC} -o $@ ${CFLAGS} ${FORK6OBJS}

pipe5bad: ${PIPE5BADOBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE5BADOBJS}

clean:
	-rm -f ${ALLPROGS} ${FORK6OBJS} ${PIPE1OBJS} ${PIPE3OBJS} \
	    ${PIPE4OBJS} ${PIPE5OBJS} ${PIPE5BADOBJS} 

.PHONY: all clean
