NAME=lisp
OFILES=\
	jmp.o\
	elem.o\
	eval.o\
	gc.o\
	main.o\
	parser.o\
	str.o

AS=$(CC) -c
CFLAGS=-c -Wall -g -O0

all: $(NAME)

%.o: %.S
	$(AS) $*.S -o $@

%.o: %.c
	$(CC) $(CFLAGS) $*.c

$(NAME): $(OFILES)
	$(CC) -o $(NAME) $(OFILES)
	
$(OFILES): dat.h fn.h

clean:
	rm -f $(NAME) $(OFILES)
