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
	switch(TYPE(obj)){
	case CELL:
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
    CASE(INT, "%d", obj->value);
	CASE(STRING, "\"%s\"", obj->beg);
    CASE(SYMBOL, "%s", obj->sym);
	CASE(ENV,    "Env");
	CASE(LAMBDA, "<lambda>");
	CASE(FUNC,   "<func>");
#undef CASE
    default:
		printf("!!error!!");
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
