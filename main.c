#include "dat.h"
#include "fn.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>

jmp_buf  recover_stack;

static void
SExprint(Object **obj)
{
	switch(TYPE(*obj)){
	case CELL:
		printf("(");
		SExprint((*obj)->car);
		printf(" . ");
		SExprint((*obj)->cdr);
		printf(")");
		break;
	case INT:
		printf("%ld", (*obj)->value);
		break;
	case SYMBOL:
		printf("%s", (*obj)->sym);
		break;
	case ENV:
		printf("<");
		for(Object **c = (*obj)->vars; c!=Nil; c=(*c)->cdr){
			printf("'%s', ", (*(*(*c)->car)->car)->sym);
		}
		printf(">");
		break;
	case LAMBDA:
		printf("<lambda>");
		goto func;
	case FUNC:
		printf("<func>");
func:
		printf("<");
		SExprint((*obj)->params);
		SExprint((*obj)->body);
		SExprint((*obj)->env);
		printf(">");
		break;
#undef CASE
	default:
		printf("!!error!!");
	}
}

void
print_expr(Object **obj)
{
	SExprint(obj);
	printf("\n");
}

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
error_expr(char *msg, Object **obj)
{
	fprintf(stderr, "Error => %s\n", msg);
	fprintf(stderr, "====== Expr ======\n");
	print_expr(obj);
	longjmp(recover_stack, 1);
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
	longjmp(recover_stack, 1);
}

int
main(int argc, char *argv[])
{
	init_gc(3000);
	init_predefined();
	
	if(setjmp(recover_stack) == 1){
		skip_line();
	}
	for(;;){
		Object **obj = next_expr();
		obj = eval(root_env, obj);
		print_expr(obj);
	}
}
