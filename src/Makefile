# #####################################################################
#                              Makefile
# #####################################################################

# Executable's name
NAME	=	carta-genius
# Name of source files
SRC	=	main.c		\
		conf.c		\
		document.c	\
		paper.c		\
		card.c		\
		expr.c		\
		text_define.c

# #####################################################################

# Paths to header files
IPATH	=	-I. -I../include
IPATH_MACOSX =	$(IPATH) -I/Developer/Headers/FlatCarbon
# Path to libraries and lib's names
LDPATH	=	-L. -L../lib -ly -lpdf -lm
# Compiler options
EXEOPT	=	-O # -g for debug

# #####################################################################

CC	=	gcc
RM	=	/bin/rm -f
OBJS	=	$(SRC:.c=.o)

# Objects compilation options
CFLAGS	=	-pedantic -Wall -Wmissing-prototypes \
		  -Wno-long-long $(IPATH) -D_GNU_SOURCE \
		  -D_LARGEFILE_SOURCE -D_THREAD_SAFE -std=c9x -O
CFLAGS_CYGWIN =	-Wall -Wmissing-prototypes -Wno-long-long $(IPATH) \
		  -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_THREAD_SAFE
CFLAGS_MACOSX =	$(CFLAGS) $(IPATH_MACOSX)

# Link options
LDFLAGS	=		$(EXEOPT) $(LDPATH)
LDFLAGS_MACOSX =	$(LDFLAGS) -framework CoreServices -framework ApplicationServices

# #####################################################################

.PHONY: static dynamic cygwin macosx clean cygclean all cygall

dynamic: $(OBJS) $(SRC)
	$(CC) $(OBJS) $(LDFLAGS) -o $(NAME)
	strip $(NAME)
	if [ ! -d ../bin ]; then mkdir ../bin; fi
	cp $(NAME) ../bin

static: $(OBJS) $(SRC)
	$(CC) $(OBJS) ../lib/libpdf.a $(LDFLAGS) -o $(NAME)
	strip $(NAME)
	if [ ! -d ../bin ]; then mkdir ../bin; fi
	cp $(NAME) ../bin

cygwin: $(SRC)
	$(CC) $(CFLAGS_CYGWIN) -c $(SRC)
	$(CC) $(OBJS) $(LDFLAGS) -o $(NAME).exe
	strip $(NAME).exe
	if [ ! -d ../bin ]; then mkdir ../bin; fi
	cp $(NAME).exe ../bin

macosx: $(SRC)
	$(CC) $(CFLAGS_MACOSX) -c $(SRC)
	$(CC) $(OBJS) ../lib/libpdf.a $(LDFLAGS_MACOSX) -o $(NAME)
	strip $(NAME)
	if [ ! -d ../bin ]; then mkdir ../bin; fi
	cp $(NAME) ../bin

clean:
	$(RM) ../bin/$(NAME) $(NAME) $(OBJS) *~

cygclean:
	$(RM) ../bin/$(NAME).exe $(NAME).exe $(OBJS) *~

all: clean dynamic

cygall: cygclean cygwin

.c.o:
	$(CC) $(CFLAGS) -c $<

