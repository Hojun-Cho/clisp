 /* elem */
void add_variable(Object *sym, Object *val, Object *env);
void add_primitive(Object *sym, Primitive fn, Object *env);
void init_predefined(void);

/* parser */
Object* next_expr(void);
void print_expr(Object *obj);
void skip_line(void);

/* memory */
void* xalloc(int sz);
Object* new_int(long val);
Object* new_cons(Object *car, Object *cdr);
Object* new_symbol(char *sym);
Object* new_acons(Object *x, Object *y, Object *z);
Object* new_env(Object *vars, Object *up);
Object* new_primitve(Primitive fn);
Object* new_function(Object *env, enum Obj_Type type, Object *params, Object *body);
void init_gc(int);
void gc_run(void);

/* error */
void panic(char *fmt, ...);
void error(char *fmt, ...);
void error_expr(char *msg, Object *obj);

/* eval */
Object* eval(Object *env, Object *obj);
Object* push_env(Object *env, Object *vars, Object *args);
void init_primitive(void);

/* bin */
Bin* new_bin(int size, int n);
void* bin_alloc(Bin *bin);
void bin_free(Bin *bin, void *);

