# #####################################################################
#                              Makefile
# #####################################################################

# Library's full name (libNAME.a)
NAME      =	liby.a

# Name of source files (names.c)
SRC       =	ymalloc.c	\
		ystr.c		\
		yvect.c		\
		ylog.c		\
		ysax.c		\
		ydom.c		\
		ydom_xpath.c	\
		yvalue.c

# Name of header files (names.h)
HEADS     =	ydefs.h		\
		yerror.h	\
		ydom.h		\
		ylog.h		\
		ysax.h		\
		ystr.h		\
		yvalue.h	\
		yvect.h

# #####################################################################

# Paths to header files
IPATH   =       -I.
# Path to libraries and lib's names
LDPATH  =       -L.
# Compiler options
EXEOPT  =       -O # -g for debug

# #####################################################################

CC      =	gcc
RM      =	/bin/rm -f
OBJS    =	$(SRC:.c=.o)

# Objects compilation options
CFLAGS  =	-pedantic -Wall -Wmissing-prototypes \
		  -Wno-long-long $(IPATH) -D_GNU_SOURCE \
		  -D_LARGEFILE_SOURCE -D_THREAD_SAFE -fPIC -std=c9x -O
CFLAGS_CYGWIN =	-Wall -Wmissing-prototypes -Wno-long-long $(IPATH) \
		  -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_THREAD_SAFE

# Link options
LDFLAGS =	$(EXEOPT) $(LDPATH)

# #####################################################################

.PHONY: headers cygwin clean all cygall doc docclean

$(NAME): $(OBJS) $(SRC) headers
	ar -r $(NAME) $(OBJS)
	ranlib $(NAME)
	cp $(NAME) ..

headers:
	cp $(HEADS) ../../include

cygwin: $(SRC) headers
	$(CC) $(CFLAGS_CYGWIN) -c $(SRC)
	ar -r $(NAME) $(OBJS)
	ranlib $(NAME)
	cp $(NAME) ..

clean:
	$(RM) $(NAME) $(OBJS) ../$(NAME) *~
	if [ -d ../../include ]; then cd ../../include; $(RM) $(HEADS); fi

all: clean $(NAME)

cygall: clean cygwin

doc:	# needs the HeaderBrowser program
	headerbrowser $(HEADS)

docclean:
	$(RM) -r hbresult-html

.c.o:
	$(CC) $(CFLAGS) -c $<
