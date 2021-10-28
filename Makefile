SUBDIRS = chapter1 chapter2 lab

all clean:
	for dir in ${SUBDIRS}; do make -C $$dir $@; done

.PHONY: all clean
