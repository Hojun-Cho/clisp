#include <stdio.h>
/* parser.c */
Object* nextexpr(FILE*);
void skipline(FILE*);

/* repl.c */
void lispmain(char *argv[]);

/* eval.c */
Object* eval(Object *env, Object *expr);

/* new */
Object* newint(GC *,long);
Object* newcons(GC *,Object*,Object*);
Object* newenv(GC *gc, Object *frames, Object *bp, Object *sp);
Object* newblock(GC *gc, Object* tag, Object *up, Object *body, void *jmp);
Object* newframe(GC *gc, Object* tag, Object *local, Object *up, Object *block);
Object* newacons(GC *,Object*, Object*, Object*);
Object* newsymbol(GC *,char*, int);
Object* newstr(GC *,int);
Object* newfn(GC *,Object *frame, Object *params, Object *body, enum OType type);

/* gc.c */
GC* newgc(void *top, int cap);
void gcrun(GC *);
void* gcalloc(GC *gc, int sz);
void printgc(char *, GC *);

/* str.c */
Object* strputc(Object*, int);
Object* strputs(Object*, Object*);
int strequal(Object*, Object*);
void strinit(Object *s, Object *str);

/* error.c */
void panic(char *fmt, ...);
void error(char *fmt, ...);

/* builtin */
Bltinfn bltinlookup(Object *obj);
void printexpr(Object *obj);
