NAME=lisp
OFILES=\
	bin.o\
	env.o\
	eval.o\
	gc.o\
	main.o\
	parser.o

AS=$(CC) -c
CFLAGS=-c -Wall -g -O0

all: $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $*.c

$(NAME): $(OFILES)
	$(CC) -o $(NAME) $(OFILES)
	
$(OFILES): dat.h fn.h

clean:
	rm -f $(NAME) $(OFILES)
