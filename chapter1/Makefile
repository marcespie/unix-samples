ALLPROGS = fork1 fork2 fork3 fork3bad fork3bad2 fork3pgrp fork4 fork5 fork6 \
	pipe1 pipe1bis pipe2 pipe3 pipe4 pipe4bad server1 server2 \
	server3 server4 server5 server6 server7 server8 server9buggy \
	server9 prefork1 prefork2 prefork3 chat1


FORK6OBJS = myfuncs.o fork6.o
PIPE1OBJS = myfuncs.o pipe1.o
PIPE1BISOBJS = myfuncs.o pipe1bis.o
PIPE2OBJS = myfuncs.o pipe2.o
PIPE3OBJS = myfuncs.o pipe3.o
PIPE4OBJS = myfuncs.o pipe4.o
PIPE4BADOBJS = myfuncs.o pipe4bad.o
SERVER1OBJS = myfuncs.o server1.o
SERVER1LOCALOBJS = myfuncs.o server1local.o
SERVER2OBJS = myfuncs.o server2.o
SERVER3OBJS = myfuncs.o server3.o
SERVER4OBJS = myfuncs.o server4.o
SERVER5OBJS = myfuncs.o server5.o
SERVER6OBJS = myfuncs.o server6.o
SERVER7OBJS = myfuncs.o server7.o
SERVER8OBJS = myfuncs.o server8.o
SERVER9OBJS = myfuncs.o server9.o
SERVER9BUGGYOBJS = myfuncs.o server9buggy.o
CHAT1OBJS = myfuncs.o chat1.o
PREFORK1OBJS = myfuncs.o prefork1.o
PREFORK2OBJS = myfuncs.o prefork2.o
PREFORK3OBJS = myfuncs.o prefork3.o

all: ${ALLPROGS} MANIFEST

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

server1local: ${SERVER1LOCALOBJS}
	${CC} -o $@ ${CFLAGS} ${SERVER1LOCALOBJS}

server2: ${SERVER2OBJS}
	${CC} -o $@ ${CFLAGS} ${SERVER2OBJS}

server3: ${SERVER3OBJS}
	${CC} -o $@ ${CFLAGS} ${SERVER3OBJS}

server4: ${SERVER4OBJS}
	${CC} -o $@ ${CFLAGS} ${SERVER4OBJS}

server5: ${SERVER5OBJS}
	${CC} -o $@ ${CFLAGS} ${SERVER5OBJS}

server6: ${SERVER6OBJS}
	${CC} -o $@ ${CFLAGS} ${SERVER6OBJS}

server7: ${SERVER7OBJS}
	${CC} -o $@ ${CFLAGS} ${SERVER7OBJS}

server8: ${SERVER8OBJS}
	${CC} -o $@ ${CFLAGS} ${SERVER8OBJS}

server9buggy: ${SERVER9BUGGYOBJS}
	${CC} -o $@ ${CFLAGS} ${SERVER9BUGGYOBJS}

server9: ${SERVER9OBJS}
	${CC} -o $@ ${CFLAGS} ${SERVER9OBJS}

chat1: ${CHAT1OBJS}
	${CC} -o $@ ${CFLAGS} ${CHAT1OBJS}

prefork1: ${PREFORK1OBJS}
	${CC} -o $@ ${CFLAGS} ${PREFORK1OBJS}

prefork2: ${PREFORK2OBJS}
	${CC} -o $@ ${CFLAGS} ${PREFORK2OBJS}

prefork3: ${PREFORK3OBJS}
	${CC} -o $@ ${CFLAGS} ${PREFORK3OBJS}

clean:
	-rm -f ${ALLPROGS} MANIFEST *.o

MANIFEST: ${ALLPROGS:=.c}
	/bin/sh ../comment2MANIFEST ${ALLPROGS:=.c} >MANIFEST.tmp
	mv MANIFEST.tmp MANIFEST

.PHONY: all clean
