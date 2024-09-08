#include "dat.h"
#include "fn.h"
#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>

jmp_buf err;
GC *gc;

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
	longjmp(err, 1);
	exit(1);
}

static void
SExprint(Object *obj)
{
	if(obj == 0)
		return;
	switch(obj->type){
	default:
		return;
	case OCELL:
		printf("(");
		SExprint(obj->car);
		printf(" . ");
		SExprint(obj->cdr);
		printf(")");
		break;
	case OINT:
		printf("%ld", obj->num);
		break;
	case OIDENT:
		printf("%s", obj->beg);
		break;
	case OSTRING:
		printf("\"%s\"", obj->beg);
		break;
	case OBLTIN:
	case OSYMBOL:
		printf("%s", obj->beg);
		break;
	case OENV:
		printf("<env>");
		SExprint(obj->vars);
		break;
	case OMACRO:
		printf("<macro>");
		goto func;
	case OLAMBDA:
		printf("<lambda>");
		goto func;
	case OFUNC:
		printf("<func>");
func:
		printf("<");
		SExprint(obj->params);
		SExprint(obj->body);
		printf(">");
		break;
	}
}

void
printexpr(Object *obj)
{
	SExprint(obj);
	printf("\n");
}

static void
loop(Object *env, FILE *f)
{
	if(setjmp(err) == 1){
		if(feof(f))
			return;
		skipline(f);
	}
	while(1){
		printf(">> ");
		Object *res = nextexpr(f);
		res = eval(env, res);
		printexpr(res);
		printgc("status", gc);
	}
}

int
main(int argc, char *argv[])
{
	gc = newgc(&argc, 400);
	Object *env = newenv(gc, &Nil, &Nil, &Nil);
	for(int i = 1; i < argc; ++i){
		FILE *f = fopen(argv[i], "r");
		if(f == 0)
			panic("can't open %s", argv[i]);
		loop(env, f);
	}
	loop(env, stdin);
}
