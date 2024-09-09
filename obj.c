#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <string.h>

static Object*
newobj(GC *gc, enum OType type, int sz)
{
	Object *obj = gcalloc(gc, sizeof(Object) + sz);
	obj->type = type;
	return obj;
}

Object*
newint(GC *gc, long val)
{
	Object *obj = newobj(gc, OINT, 0);
	obj->num = val;
	return obj;
}

Object*
newcons(GC *gc, Object *car, Object *cdr)
{
	Object *obj = newobj(gc, OCELL, 0);
	obj->car = car;
	obj->cdr = cdr;
	return obj;
}

Object*
newenv(GC *gc, Object* name, Object *vars, Object *up)
{
	Object *obj = newobj(gc, OENV, 0);
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
newfn(GC *gc, Object *env, Object *params, Object *body, enum OType type)
{
	Object *fn = newobj(gc, type, 0);
	fn->type = type;
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
		&Define, &Setq, &Eq, &If, &Defn, &Macro, &Progn,
		&Bquote, &Comma,
	};
	for(int i = 0; i < sizeof(syms)/sizeof(syms[0]); ++i){
		Object *c = syms[i];
		if(strlen(c->beg)==len && memcmp(c->beg, str, len) == 0)
			return c;
	}
	Object *obj = newobj(gc, OIDENT, len + 1);
	obj->ptr = obj->beg = (char*)&obj[1];
	obj->end = obj->beg + len;
	memcpy(obj->beg, str, len + 1);
	obj->ptr += len;
	return obj;
}

Object*
newstr(GC *gc, int len)
{
	Object *obj = newobj(gc, OSTRING, len + 1);
	obj->ptr = obj->beg = (char*)&obj[1];
	obj->end = obj->beg + len;
	return obj;
}
