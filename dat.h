typedef struct GC GC;
typedef struct Object Object;
typedef Object* (*Bltinfn)(Object *env, Object *args);

enum OType
{
	ONONE,
	OBLTIN,
	OSYMBOL,
	OCELL,
	OIDENT,
	OSTRING,
	OINT,
	OFUNC,
	OMACRO,
	OENV,
};

struct Object
{
	enum OType type; /* type */
	int flag; 		/* flag */
	Object *forward;
	union{
		/* int */
		long num;
		/* cell */
		struct{
			Object *car;
			Object *cdr;
		};
		/* string & ident  & symbol */
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

extern GC *gc;
extern Object Nil;
extern Object Comma;
extern Object Bquote;
extern Object Minus;
extern Object Plus;
extern Object Mul;
extern Object Div;
extern Object Mod;
extern Object Lambda;
extern Object Car;
extern Object Cdr;
extern Object Quote;
extern Object Cons;
extern Object Define;
extern Object Progn;
extern Object Macro;
extern Object Setq;
extern Object Eq;
extern Object Ne;
extern Object If;
extern Object Ge;
extern Object Le;
extern Object Lt;
extern Object Gt;
