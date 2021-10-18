#ifndef CONFIG_H
#  define CONFIG_H

#  ifdef __OpenBSD__
#    define HAVE_REALLOCARRAY	1
#  endif

#  ifndef HAVE_REALLOCARRAY
#    define HAVE_REALLOCARRAY	0
#  endif

#endif
