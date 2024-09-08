#include "dat.h"
#include "fn.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>

jmp_buf *errptr;

void
panic(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(1);
}

void
error(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "ERROR => ");
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	longjmp(*errptr, 1);
	exit(1);
}

void
repl(Object *env, FILE *f, char *pre)
{
	jmp_buf err;
	errptr = &err;
	if(setjmp(err) == 1){
		if(feof(f))
			return;
		skipline(f);
	}
	while(1){
		printf(pre);
		Object *res = nextexpr(f);
		res = eval(env, res);
		printexpr(res);
		printgc("status", gc);
	}
}

void 
readlibs(char *argv[], Object *env)
{
	for(;*argv; argv++){
		FILE *f = fopen(*argv, "r");
		if(f == 0)
			panic("can't open %s", *argv);
		repl(env, f, "");
		printf("\n");
	}
}
