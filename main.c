#include "dat.h"
#include "fn.h"
#include <setjmp.h>

jmp_buf *errptr;
GC *gc;

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
    jmp_buf buf;
    errptr = &buf;
	if(setjmp(buf) == 1){
		if(feof(f))
			return;
		skipline(f);
	}
	while(1){
		Object *res = nextexpr(f);
		printexpr(res);
		res = eval(env, res);
		printgc("status", gc);
		printf("=============res===========\n");
		printexpr(res);
		printf("=============env===========\n");
		printexpr(env);
		printf("===========================\n");
	}
}

int
main(int argc, char *argv[])
{
	gc = newgc(&argc, 400);
	Object *env = newenv(gc, &Nil, &Nil, &Nil);
	loop(env, stdin);
}
