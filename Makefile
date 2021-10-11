ALLPROGS = fork1 fork2 fork3 fork3bad fork3bad2 fork4 fork5 \
	pipe1 pipe2 pipe3

PIPE3OBJS = myfuncs.o pipe3.o

all: ${ALLPROGS}


pipe3: ${PIPE3OBJS}
	${CC} -o $@ ${CFLAGS} ${PIPE3OBJS}

clean:
	-rm -f ${ALLPROGS} ${PIPE3OBJS}

.PHONY: all
