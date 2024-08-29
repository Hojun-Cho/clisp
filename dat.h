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
