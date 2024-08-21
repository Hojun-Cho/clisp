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

/* env */
Object*
push_env(Object *env, Object *vars, Object *args)
{
	Object *map = Nil;
	for(;TYPE(vars) == CELL; vars = vars->cdr, args = args->cdr){
		if(TYPE(args) != CELL)
			error("Can't apply function argment dose not match");
		Object *sym = vars->car;
		Object *arg = args->car;
		map = new_acons(sym, arg, map);
	}
	if(vars != Nil)
		error("Can't apply function argment dose not match");
	return new_env(map, env);
}

void
add_variable(Object *sym, Object *val, Object *env)
{
	Object *vars = env->vars;
	env->vars = new_acons(sym, val, vars);
}

static Object*
_new_symbol(char *sym)
{
	Object *obj = new_object(SYMBOL);
	memcpy(obj->sym, sym, strlen(sym) + 1);
	return obj;
}

static void
_init_const_vars(void)
{
	symbols = Nil = _new_symbol("nil");	
	True = new_symbol("true"); 
	False = new_symbol("false");
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
	_init_const_vars();
	entry_root(symbols);
	root_env = new_env(Nil, Nil);
	entry_root(root_env);
	_append_to_env();
	init_primitive();
}
