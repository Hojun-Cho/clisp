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

/* remove all frames except Top */
static void
clearenv(Object *env)
{
	env->bp->cdr = &Nil;
	env->bp->car->block = &Top; 
	env->sp = env->bp;
	env->retval = &Nil;
}

static void
repl(Object *env, FILE *f, char *pre)
{
	jmp_buf err;
	errptr = &err;
	if(setjmp(err) == 1){
		if(feof(f))
			exit(1);
		clearenv(env);
		skipline(f);
	}
	while(1){
		printf(pre);
		Object *res = nextexpr(f);
		res = eval(env, res);
		printexpr(res);
	}
}

static void
readlib(FILE *f, Object *env)
{
	jmp_buf buf;
	errptr = &buf;
	if(setjmp(buf) == 1)
		return;
	while(1){
		eval(env, nextexpr(f));
	}
	panic("unreachable");
	errptr = 0;
}

void
lispmain(char *argv[])
{
	Object *frame = newframe(gc, &Top, &Nil, &Nil, &Top);
	Object *cons = newcons(gc, frame, &Nil);
	Object *env = newenv(gc, cons, cons, cons);
	for(; *argv; ++argv){
		FILE *f = fopen(*argv, "r");
		if(f == 0)
			panic("can't open %s'", *argv);
		readlib(f, env);
		fclose(f);
	}
	repl(env, stdin, ">> ");
}