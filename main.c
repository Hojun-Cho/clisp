#include "dat.h"
#include "fn.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define USING(x) ({void *xxx = (x);})

extern int psetjmp(void*);
extern void plongjmp(void*, int);

enum { REG_RIP = 7, REG_RSP = 6, };
typedef void *jmp_buf[10];

jmp_buf root_stack, recover_stack;
void *workspace;
void *stack_top;
void *stack_bot;

static void
SExprint(Object *obj)
{
	if(obj == 0){
		printf("Nil");
		return;
	}
	switch(TYPE(obj)){
	case Obj_Cell:
		printf("(");
		SExprint(obj->car);
		printf(" . ");
		SExprint(obj->cdr);
		printf(")");
		return;
#define CASE(type, ...)                         \
    case type:                                  \
        printf(__VA_ARGS__);                    \
        return
    CASE(Obj_Int, "%d", obj->value);
	CASE(Obj_String, "\"%s\"", obj->beg);
    CASE(Obj_Symbol, "%s", obj->sym);
	CASE(Obj_Map,    "Map");
	CASE(Obj_Env,    "Env");
    CASE(Obj_True,   "true");
    CASE(Obj_False,  "false");
	CASE(Obj_Lambda, "<lambda>");
	CASE(Obj_Func,   "<func>");
	CASE(Obj_Nil,  "Nil");
#undef CASE
    default:
        error("SExpr=> print: Unknown type %d", TYPE(obj));
    }
}

void
print_expr(Object *obj)
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
error_expr(char *msg, Object *obj)
{
	fprintf(stderr, "Error => %s\n", msg);
	fprintf(stderr, "====== Expr ======\n");
	print_expr(obj);
	plongjmp(recover_stack, 1);
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
	plongjmp(recover_stack, 1);
}

static void
_main(void)
{
	init_gc();
	init_predefined();

	if(psetjmp(recover_stack) == 1){
		skip_line();
	}
	for(;;){
		Object *obj = next_expr();
		obj = eval(root_env, obj);
		print_expr(obj);
	}
}

int
main(int argc, char *argv[])
{
	uint8_t *ptr = workspace = xalloc(STACK_SIZE);
	/* sub 8 because alignment of 16 */
	root_stack[REG_RSP] = ptr + STACK_SIZE - (64 - 8);
	root_stack[REG_RIP] = _main;
	stack_bot = ptr;
	stack_top = &ptr[STACK_SIZE];
	plongjmp(root_stack, 0);
}
