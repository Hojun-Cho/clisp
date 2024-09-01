/* parser.c */
Object* nextexpr(void);
void skipline(void);

/* eval.c */
Object* eval(Object *env, Object *expr);

/* new */

Object* newint(long);
Object* newcons(Object*,Object*);
Object* newenv(Object*name, Object *vars, Object *up);
Object* newacons(Object*, Object*, Object*);
Object* newsymbol(char*, int);
Object* newstr(int);
Object* newfn(Object *env, Object *params, Object *body);

/* gc.c */
void gcstatus(void);
Object* newobj(enum OType);
void* gcalloc(int);
void* gcralloc(void*, int);
void gcinit(void *top, int cap);
void gcrun(void);

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
