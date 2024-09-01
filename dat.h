#include <stdint.h>

typedef uintptr_t u64;
typedef struct Object Object;
typedef Object* (*Bltinfn)(Object *env, Object *args);
typedef struct Object Object;

enum OType
{
	OERROR,
	OCELL,
	OSYMBOL,
	OIDENT,
	OINT,
	OSTRING,
	OLAMBDA,
	OBLTIN,
	OFUNC,
	OENV,
};

struct Object
{
	enum OType type; /* type */
	int flag; 		/* flag */
	Object *next;   /* for gc */
	Object *forward;
	union{
		/* int */
		long num;
		/* cell */
		struct{
			Object *car;
			Object *cdr;
		};
		/* string & ident */
		char *sym;
		struct{
			char *beg;
			char *ptr;
			char *end;
		};
		/* function */
		struct{
			Object *params;
			Object *body;
			Object *env;
		};
		/* env */ 
		struct{
			Object *name;
			Object *up;
			Object *vars;
		};
	};
};

/*
 *0  ~ 64  : for object 
 *64 ~ 100 : for string 
 */
typedef struct
{
	int running;
	void *memory;
	u64 cap;
	u64 using;
	u64 top;
	/* objects */
	struct{
		Object *objs;
		u64 ob;
		u64 oe;
		u64 op;
		Object *freed;
	};
	/* string  */
	struct{
		u64 sb;
		u64 se;
	};
}GC;

extern GC *gc;
extern Object Nil;
extern Object True;
extern Object False;
extern Object Minus;
extern Object Plus;
extern Object Lambda;
extern Object Car;
extern Object Cdr;
extern Object Quote;
extern Object Cons;
extern Object Define;
extern Object Setq;
