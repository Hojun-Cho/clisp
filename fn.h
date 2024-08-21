 /* elem */
void add_variable(Object *sym, Object *val, Object *env);
void add_primitive(Object *sym, Primitive fn, Object *env);
void init_predefined(void);

/* String */
Object* new_string(char *ptr);
void str_putc(Object *s, int c);
void str_puts(Object *s, char *ptr);
int str_len(Object *a);
int str_cmp(Object *a, Object *b);

/* parser */
Object* next_expr(void);
void print_expr(Object *obj);
void skip_line(void);

/* memory */
void* xalloc(int sz);
Object* new_int(int val);
Object* new_cons(Object *car, Object *cdr);
Object* new_symbol(char *sym);
Object* new_acons(Object *x, Object *y, Object *z);
Object* new_env(Object *vars, Object *up);
Object* new_primitve(Primitive fn);
Object* new_function(Object *env, enum Obj_Type type, Object *params, Object *body);
void init_gc(int siz1000e);
void gc_run(void);

/* error */
void panic(char *fmt, ...);
void error(char *fmt, ...);
void error_expr(char *msg, Object *obj);

/* eval */
Object* eval(Object *env, Object *obj);
Object* push_env(Object *env, Object *vars, Object *args);
void init_primitive(void);
