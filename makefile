NAME=lisp
OFILES=\
	bltin.o\
	eval.o\
	main.o\
	gc.o\
	obj.o\
	str.o\
	parser.o

AS=$(CC) -c
CFLAGS=-c  -g -O2 -Wall -std=c99

all: $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $*.c

$(NAME): $(OFILES)
	$(CC) -o $(NAME) $(OFILES)
	
$(OFILES): dat.h fn.h

clean:
	rm -f $(NAME) $(OFILES)
