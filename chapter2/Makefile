# we gain a library !
ALLPROGS = 

all: ${ALLPROGS} MANIFEST libfoo.a

LIBSRC=	reallocarray.c
LIBOBJS = ${LIBSRC:.c=.o}

libfoo.a: ${LIBOBJS}
	rm -f $@
	ar cr $@ ${LIBOBJS}
	ranlib $@

${LIBOBJS}: foo.h

clean:
	-rm -f ${ALLPROGS} libfoo.a MANIFEST *.o

MANIFEST: ${ALLPROGS:=.c}
	/bin/sh ../comment2MANIFEST ${ALLPROGS:=.c} >MANIFEST.tmp
	mv MANIFEST.tmp MANIFEST

.PHONY: all clean
