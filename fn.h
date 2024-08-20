 /* elem */
void add_variable(Object *sym, Object *val, Object *env);
void add_primitive(char *name, Primitive fn, Object *env);
void init_predefined(void);

/* String */
Object* new_string(char *ptr, int fixed);
void str_putc(Object *s, int c);
void str_puts(Object *s, char *ptr);
int str_len(Object *a);
int str_cmp(Object *a, Object *b);

/* Map */
Object* new_map(int cap, int (*cmp)(void*,void*), int (*hash)(Object*));
Object* map_get(Object *map, void *key);
void map_set(Object *map, void *key, Object *val);
void map_iterate(Object *map, void (*fn)(Object *));
void del_map(Object *obj);

/* parser */
Object* next_expr(void);
void print_expr(Object *obj);
void skip_line(void);
Object* reverse(Object *p);

/* memory */
void* xalloc(int sz);
Object* new_object(enum Obj_Type type);
Object* new_int(int val);
Object* new_cons(Object *car, Object *cdr);
Object* new_symbol(char *sym);
Object* new_acons(Object *x, Object *y, Object *z);
Object* new_env(Object *vars, Object *up);
Object* new_primitve(Primitive fn);
Object* new_function(Object *env, enum Obj_Type type, Object *params, Object *body);
void entry_root(Object *obj);
void init_gc(void);
void gc_run(void);

/* error */
void panic(char *fmt, ...);
void error(char *fmt, ...);
void error_expr(char *msg, Object *obj);

/* eval */
Object* eval(Object *env, Object *obj);
Object* push_env(Object *env, Object *vars, Object *args);
Object* fn_plus(Object *env, Object *args);
Object* fn_minus(Object *env, Object *args);
Object* fn_lambda(Object *env, Object *args);
