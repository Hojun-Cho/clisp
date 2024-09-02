#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <string.h>

Object*
newint(GC *gc, long val)
{
	Object *obj = newobj(gc, OINT);
	obj->num = val;
	return obj;
}

Object*
newcons(GC *gc, Object *car, Object *cdr)
{
	Object *obj = newobj(gc, OCELL);
	obj->car = car;
	obj->cdr = cdr;
	return obj;
}

Object*
newenv(GC *gc, Object* name, Object *vars, Object *up)
{
	Object *obj = newobj(gc, OENV);
	obj->name = name;
	obj->up = up;
	obj->vars = vars;
	return obj;
}

Object*
newacons(GC *gc, Object *x, Object *y, Object *z)
{
	Object *cons = newcons(gc, x, y);
	return newcons(gc, cons ,z);
}

Object*
newfn(GC *gc, Object *env, Object *params, Object *body)
{
	Object *fn = newobj(gc, OFUNC);
	fn->params = params;
	fn->body = body;
	fn->env = env; 
	return fn;
}

Object*
newsymbol(GC *gc, char *str, int len)
{
	static Object *syms[] = {
		&Nil,  &Minus, &Plus, &Mul, &Mod, &Div, &Ge, &Le,
		&Lt, &Gt, &Ne, &Lambda, &Car, &Cdr, &Quote, &Cons,
		&Define, &Setq, &Eq, &If,
	};
	for(int i = 0; i < sizeof(syms)/sizeof(syms[0]); ++i){
		Object *c = syms[i];
		if(strlen(c->sym)==len && memcmp(c->sym, str, len) == 0)
			return c;
	}
	Object *obj = newobj(gc, OIDENT);
	obj->beg = gcalloc(gc, len + 1);
	obj->end = obj->ptr = obj->beg + len;	
	memcpy(obj->beg, str, len+1);
	return obj;
}

Object*
newstr(GC *gc, int len)
{
	Object *obj = newobj(gc, OSTRING); 
	obj->ptr = obj->beg = gcalloc(gc, len + 1);
	obj->end = obj->beg + len;
	return obj; 
}
