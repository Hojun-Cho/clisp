#define DEFAULT_OBJS_CAP 64
#define STR_DEFAULT_LEN 16
#define SYMBOL_MAX_LEN 64
#define STACK_SIZE 8192 * 4

#define TYPE(x) ((x)->type & 0b11111111)

enum Obj_Type
{
	NONE,
	INT,
	CELL,
	SYMBOL,
	PRIM,
	FUNC,
	LAMBDA,
	ENV,

	Obj_Marked = 1 << 10,
	Obj_Using = 1 << 16,
};

enum
{ 
	REG_RIP = 7,
	REG_RSP = 6,
};

typedef void *jmp_buf[10];
typedef struct Object Object;
typedef Object** (*Primitive)(Object **env, Object **args);
typedef struct Slot Slot;

struct Object
{
	int size;
	int type;
	union{
		/* int */
		long value;
		/* cell */
		struct{
			Object **car;
			Object **cdr;
		};
		/* symbol */
		char *sym;
		/* primitive */
		Primitive fn;
		/* function */
		struct{
			Object **params;
			Object **body;
			Object **env;
		};
		/* env */ 
		struct{
			Object **vars;
			Object **up;
		};
	};
};

/* const */
extern Object** True;
extern Object** False;
extern Object** Nil;
extern Object** Plus, **Minus, **Lambda, **Car, **Cdr, **Quote, **Cons, **Define;

/* stack */
extern void *workspace;
extern void *stack_top;
extern void *stack_bot;

/* root objects */
extern Object **symbols;
extern Object **root_env;

/* stack */
extern jmp_buf root_stack, recover_stack;
