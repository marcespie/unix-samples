# we gain a library !
ALLPROGS = chat2

all: ${ALLPROGS} MANIFEST libfoo.a

LIBSRC=	reallocarray.c strlcpy.c strlcat.c emem.c create_servers.c \
	safe_write.c
LIBOBJS = ${LIBSRC:.c=.o}
LINKFLAGS = -L. -lfoo

CHAT2OBJS = chat2.o

libfoo.a: ${LIBOBJS}
	rm -f $@
	ar cr $@ ${LIBOBJS}
	ranlib $@

${LIBOBJS}: foo.h

chat2: ${CHAT2OBJS} libfoo.a
	${CC} -o $@ ${CFLAGS} ${CHAT2OBJS} ${LINKFLAGS}

clean:
	-rm -f ${ALLPROGS} libfoo.a MANIFEST *.o

MANIFEST: ${ALLPROGS:=.c}
	/bin/sh ../comment2MANIFEST ${ALLPROGS:=.c} >MANIFEST.tmp
	mv MANIFEST.tmp MANIFEST

.PHONY: all clean
