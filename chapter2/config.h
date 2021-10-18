// XXX I know that OpenBSD has those functions
// are they elsewhere ? you tell me !
#ifndef CONFIG_H
#  define CONFIG_H

#  ifdef __OpenBSD__
#    define HAVE_REALLOCARRAY	1
#    define HAVE_STRLCPY	1
#    define HAVE_STRLCAT	1
#  endif


#  ifndef HAVE_REALLOCARRAY
#    define HAVE_REALLOCARRAY	0
#  endif

#  ifndef HAVE_STRLCPY
#    define HAVE_STRLCPY	0
#  endif

#  ifndef HAVE_STRLCAT
#    define HAVE_STRCAT		0
#  endif

#endif
