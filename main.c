#include "dat.h"
#include "fn.h"

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

int
main(int argc, char *argv[])
{
	gc = newgc(&argc, 400);
	Object *env = newenv(gc, &Nil, &Nil, &Nil);
	readlibs(argv + 1, env);
	repl(env, stdin, ">> ");
}
