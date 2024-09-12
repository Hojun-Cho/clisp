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
	OBLOCK,
	OFRAME,
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
		/* Block */
		struct{
			Object *_tag;
			Object *_up;
			Object *_body;
			void *jmp;
		};
		/* function */
		struct{
			Object *params;
			Object *frame; /* running frame */
			Object *body;
			Object *block;
		};
		/* Frame */
		struct{
			Object *tag;    /* Frame enter name  */
			Object *up;
			Object *local;  /* local vars  */
		};
		/* Env */
		struct{
			Object *frames;
			Object *bp;
			Object *sp;      /* current */
			Object *retval; /* for return-from */
		};
	};
};

extern GC *gc;
extern Object Nil;
extern Object Top;
extern Object Block;
extern Object RetFrom;
extern Object Comma;
extern Object Splice;
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
extern Object Let;
extern Object Eq;
extern Object Not;
extern Object Ne;
extern Object If;
extern Object Ge;
extern Object Le;
extern Object Lt;
extern Object Gt;
