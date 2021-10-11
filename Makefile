ALLPROGS = fork1 fork2 fork3 fork3bad fork3bad2 fork4

all: ${ALLPROGS}

clean:
	-rm ${ALLPROGS}

.PHONY: all
