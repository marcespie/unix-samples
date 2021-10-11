ALLPROGS = fork1 fork2 fork3 fork3bad fork3bad2 fork4 fork5 \
	pipe1 pipe2

all: ${ALLPROGS}

clean:
	-rm -f ${ALLPROGS}

.PHONY: all
