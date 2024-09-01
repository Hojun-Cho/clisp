#include "dat.h"
#include "fn.h"
#include <setjmp.h>
#include <stdio.h>

jmp_buf err;
GC *gc;

static void
SExprint(Object *obj)
{
	if(obj == 0)
		return;
	switch(obj->type){
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
		printf("%s", obj->sym);
		break;
	case OENV:
		printf("<env>");
		break;
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
loop(Object *env)
{
	if(setjmp(err) == 1){
		skipline();
		gcrun(gc);
	}
	while(1){
		Object *res = nextexpr();
		res = eval(env, res);
		printexpr(res);
	}
}

int
main(int argc, char *argv[])
{
	gc = newgc(&argc, 4000);
	Object *env = newenv(gc, &Nil, &Nil, &Nil);
	loop(env);
}
