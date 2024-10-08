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
	case OBLOCK:
		printf("\n<block-%s>\n", obj->tag->beg);
		break;
	case OFRAME:
		printf("\n<frame-%s>\n", obj->tag->beg);
		printexpr(obj->local);
		printexpr(obj->block);
		break;
	case OENV:
		printf("<env>");
		printexpr(obj->frames);
		break;
	case OMACRO:
		printf("<macro>");
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
	*argv = "lib/lib.lisp";
	gc = newgc(&argc, 24000);
	lispmain(argv);
	panic("unreachable");
}
