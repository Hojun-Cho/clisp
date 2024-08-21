#include "dat.h"
#include "fn.h"
#include <string.h>
#include <stdlib.h>

/* Const Variables */
Object *True;
Object *False;
Object *Nil;

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
	if(TYPE(x) != SYMBOL)
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
	if(TYPE(x) != SYMBOL || TYPE(y) != SYMBOL)
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
	for(;TYPE(vars) == CELL; vars = vars->cdr, args = args->cdr){
		if(TYPE(args) != CELL)
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
	True = new_symbol("true"); 	 
	False = new_symbol("false");
	Nil = new_symbol("nil");	 
}

static void
_append_to_env(void)
{
	add_variable(True, True, root_env);
	add_variable(False, False, root_env);
	add_variable(Nil, Nil, root_env);
}

void
add_primitive(char *name, Primitive fn, Object *env)
{
	Object *sym = new_symbol(name);	
	Object *prim = new_primitve(fn);
	add_variable(sym, prim, env);
}

void
init_predefined(void)
{
	symbols = new_map(10, _symbol_key_cmp, _symbol_obj_hash);
	entry_root(symbols);
	_init_const_vars();
	root_env = new_env(Nil, Nil);
	entry_root(root_env);
	root_env->vars = new_map(10, _env_key_cmp, _env_obj_hash);
	_append_to_env();
	init_primitive();
}
