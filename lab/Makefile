ALLPROGS = pollsig1 pollsig2 pollsig3 soshutdown

all: ${ALLPROGS} MANIFEST

pollsig1: pollsig1.o
	${CC} -o $@ ${CFLAGS} pollsig1.o

pollsig2: pollsig2.o
	${CC} -o $@ ${CFLAGS} pollsig2.o

pollsig3: pollsig3.o
	${CC} -o $@ ${CFLAGS} pollsig3.o

soshutdown: soshutdown.o
	${CC} -o $@ ${CFLAGS} soshutdown.o

clean:
	-rm -f ${ALLPROGS} MANIFEST *.o

MANIFEST: ${ALLPROGS:=.c}
	/bin/sh ../comment2MANIFEST ${ALLPROGS:=.c} >MANIFEST.tmp
	mv MANIFEST.tmp MANIFEST

.PHONY: all clean
