#include "dat.h"
#include "fn.h"
#include <string.h>
#include <stdlib.h>

extern Object* fn_plus(Object *env, Object *args);
extern Object* fn_minus(Object *env, Object *args);
extern Object* fn_lambda(Object *env, Object *args);
extern Object* fn_car(Object *env, Object *list);
extern Object* fn_cdr(Object *env, Object *list);
extern Object* fn_quote(Object *env, Object *list);

/* Const Variables */
Object *True = &(Object){ .type = Obj_True };
Object *False = &(Object){ .type = Obj_False };
Object *Nil = &(Object){ .type = Obj_Nil };
Object *Dot = &(Object){ .type = Obj_Dot };
Object *Cparen = &(Object){ .type = Obj_Cparen };

/* roots */
Object *symbols;
Object *root_env;

/* for map */
static int
_symbol_key_cmp(void *_x, void *_y)
{
	Object *x = _x;
	char *y = _y;
	if(x == 0 || y == 0)
		return -1;
	if(_x == _y)
		return 0;
	if(TYPE(x) != Obj_Symbol)
		return -1;
	return strcmp(x->sym, y);
}

static int
_symbol_obj_hash(Object *x)
{
	return 0;
}

static int
_env_key_cmp(void *_x, void *_y)
{
	Object *x = _x;
	Object *y = _y;
	if(x == 0 || y == 0)
		return -1;
	if(x == y)
		return 0;
	if(TYPE(x) != Obj_Symbol || TYPE(y) != Obj_Symbol)
		return -1;
	return strcmp(x->sym, y->sym);
}

static int
_env_obj_hash(Object *x)
{
	return 0;
}

/* env */
Object*
push_env(Object *env, Object *vars, Object *args)
{
	Object *map = new_map(10, _env_key_cmp, _env_obj_hash);
	for(;TYPE(vars) == Obj_Cell; vars = vars->cdr, args = args->cdr){
		if(TYPE(args) != Obj_Cell)
			error("Can't apply function argment dose not match");
		Object *sym = vars->car;
		Object *arg = args->car;
		map_set(map, sym, arg);
	}	
	if(vars != Nil)
		error("Can't apply function argment dose not match");
	return new_env(map, env);
}

void
add_variable(Object *sym, Object *val, Object *env)
{
	map_set(env->vars, sym, val);
}

static void
_init_const_vars(void)
{
	Object *sym;
	sym = new_symbol("true"); 	add_variable(sym, True, root_env);
	sym = new_symbol("false");	add_variable(sym, False, root_env);
	sym = new_symbol("nil");	add_variable(sym, Nil, root_env);
	sym = new_symbol(".");		add_variable(sym, Dot, root_env);
}

void
add_primitive(char *name, Primitive fn, Object *env)
{
	Object *sym = new_symbol(name);	
	SET_ATOM(sym);
	Object *prim = new_primitve(fn);
	add_variable(sym, prim, env);
}

static void
_init_primitive(void)
{
	add_primitive("+", fn_plus, root_env);
    add_primitive("-", fn_minus, root_env);
     add_primitive("lambda", fn_lambda, root_env);
	 add_primitive("car", fn_car, root_env);
	 add_primitive("quote", fn_quote, root_env);
     add_primitive("cdr", fn_cdr, root_env);
	/*
     *add_primitive("cons", fn_cons, root_env);
     *add_primitive("setq", fn_setq, root_env);
     *add_primitive("setcar", fn_setcar, root_env);
     *add_primitive("while", fn_while, root_env);
     *add_primitive("gensym", fn_gensym, root_env);
	 *add_primitive("<", fn_lt, root_env);
     *add_primitive("define", fn_define, root_env);
     *add_primitive("defun", fn_defun, root_env);
     *add_primitive("defmacro", fn_defmacro, root_env);
     *add_primitive("macroexpand", fn_macroexpand, root_env);
     *add_primitive("if", fn_if, root_env);
     *add_primitive("=", fn_num_eq, root_env);
     *add_primitive("eq", fn_eq, root_env);
     *add_primitive("println", fn_println, root_env);
	 */
}

void
init_predefined(void)
{
	symbols = new_map(10, _symbol_key_cmp, _symbol_obj_hash);
	entry_root(symbols);
	root_env = new_env(Nil, Nil);
	entry_root(root_env);
	root_env->vars = new_map(10, _env_key_cmp, _env_obj_hash);

	_init_const_vars();
	_init_primitive();
}
