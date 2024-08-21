#define DEFAULT_OBJS_CAP 64
#define STR_DEFAULT_LEN 64
#define SYMBOL_MAX_LEN 64
#define STACK_SIZE 8192 * 4

#define TYPE(x) ((x)->type & 0b11111111)

enum Obj_Type
{
	NONE,
	INT,
	STRING,
	CELL,
	SYMBOL,
	PRIM,
	FUNC,
	LAMBDA,
	MAP,
	ENV,

	Obj_Marked = 1 << 10,
	Obj_Using = 1 << 16,
};

typedef struct Object Object;
typedef Object* (*Primitive)(Object *env, Object *args);
typedef struct Slot Slot;

struct Object
{
	int type;
	union{
		/* int */
		int value;
		/* cell */
		struct{
			Object *car;
			Object *cdr;
		};
		/* string */
		struct{
			char *beg;
			char *ptr;
			char *end;
			int fixed;
		};
		/* symbol */
		char *sym;
		/* primitive */
		Primitive fn;
		/* function */
		struct{
			Object *params;
			Object *body;
			Object *env;
		};
		/* frame */ 
		struct{
			Object *vars; /* actually map */
			Object *up;
		};
		/* Map object */
		struct{
			int cap;
			Slot *slots;
			int (*cmp)(void *, void *);
			int (*hash)(Object *);
		};
	};
};

/* const */
extern Object *True;
extern Object *Nil;
extern Object *False;

/* stack */
extern void *workspace;
extern void *stack_top;
extern void *stack_bot;

/* root objects */
extern Object *symbols;
extern Object *root_env;
