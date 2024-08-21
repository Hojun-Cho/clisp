#include "dat.h"
#include "fn.h"
#include <string.h>
#include <stdlib.h>

/* Const Variables */
Object *True;
Object *False;
Object *Nil;
Object *Plus, *Minus, *Lambda, *Car, *Cdr, *Quote;

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

void
add_primitive(Object *sym, Primitive fn, Object *env)
{
	Object *prim = new_primitve(fn);
	add_variable(sym, prim, env);
}

void
init_predefined(void)
{
	symbols = Nil = _new_symbol("nil");	
	True = new_symbol("true"); 
	False = new_symbol("false");
	Plus = new_symbol("+");
	Minus = new_symbol("-");
	Car = new_symbol("car");
	Cdr = new_symbol("cdr");
	Quote = new_symbol("'");
	Lambda = new_symbol("lambda");

	root_env = new_env(Nil, Nil);
	add_variable(True, True, root_env);
	add_variable(False, False, root_env);
	add_variable(Nil, Nil, root_env);

	init_primitive();
}
