/* parser.c */
Object* nextexpr(void);
void skipline(void);

/* eval.c */
Object* eval(Object *env, Object *expr);

/* new */

Object* newint(GC *,long);
Object* newcons(GC *,Object*,Object*);
Object* newenv(GC *,Object*name, Object *vars, Object *up);
Object* newacons(GC *,Object*, Object*, Object*);
Object* newsymbol(GC *,char*, int);
Object* newstr(GC *,int);
Object* newfn(GC *,Object *env, Object *params, Object *body);

/* gc.c */
Object* newobj(GC *,enum OType);
void* gcalloc(GC *,int);
void* gcralloc(GC *, void*, int);
GC* newgc(void *top, int cap);
void gcrun(GC *);

/* str.c */
void strputc(Object*, int);
void strputs(Object*, char*);
int strequal(Object*, Object*);

/* error.c */
void panic(char *fmt, ...);
void error(char *fmt, ...);

/* builtin */
Bltinfn bltinlookup(Object *obj);
void printexpr(Object *obj);
