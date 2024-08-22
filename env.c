#include "dat.h"
#include "fn.h"
#include <string.h>
#include <stdlib.h>

/* Const Variables */
Object** True;
Object** False;
Object** Nil;
Object** Plus, **Minus, **Lambda, **Car, **Cdr, **Quote;

/* roots */
Object** symbols;
Object** root_env;

/* env */
Object**
push_env(Object **env, Object **vars, Object **args)
{
	Object **map = Nil;
	for(;TYPE(*vars)==CELL; vars=(*vars)->cdr,args=(*args)->cdr){
		if(TYPE(*args) != CELL)
			error("Can't apply function argment dose not match");
		Object **sym = (*vars)->car;
		Object **arg = (*args)->car;
		map = new_acons(sym, arg, map);
	}
	if(vars != Nil)
		error("Can't apply function argment dose not match");
	return new_env(map, env);
}

void
add_variable(Object **sym, Object **val, Object **env)
{
	Object **vars = (*env)->vars;
	(*env)->vars = new_acons(sym, val, vars);
}

void
add_primitive(Object **sym, Primitive fn, Object **env)
{
	Object **prim = new_primitve(fn);
	add_variable(sym, prim, env);
}
