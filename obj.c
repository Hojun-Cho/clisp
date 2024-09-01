#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <string.h>

Object*
newint(long val)
{
	Object *obj = newobj(OINT);
	obj->num = val;
	return obj;
}

Object*
newcons(Object *car, Object *cdr)
{
	Object *obj = newobj(OCELL);
	obj->car = car;
	obj->cdr = cdr;
	return obj;
}

Object*
newenv(Object* name, Object *vars, Object *up)
{
	Object *obj = newobj(OENV);
	obj->name = name;
	obj->up = up;
	obj->vars = vars;
	return obj;
}

Object*
newacons(Object *x, Object *y, Object *z)
{
	Object *cons = newcons(x, y);
	return newcons(cons, z);
}

Object*
newfn(Object *env, Object *params, Object *body)
{
	Object *fn = newobj(OFUNC);
	fn->params = params;
	fn->body = body;
	fn->env = env; 
	return fn;
}

Object*
newsymbol(char *str, int len)
{
	static Object *syms[] = {
		&Nil, &True, &False, &Minus, &Plus,
		&Lambda, &Car, &Cdr, &Quote, &Cons, &Define, &Setq,
	};
	for(int i = 0; i < sizeof(syms)/sizeof(syms[0]); ++i){
		Object *c = syms[i];
		if(strlen(c->sym)==len && memcmp(c->sym, str, len) == 0)
			return c;
	}
	Object *obj = newobj(OIDENT);
	obj->beg = gcalloc(len + 1);
	obj->end = obj->ptr = obj->beg + len;	
	memcpy(obj->beg, str, len+1);
	return obj;
}

Object*
newstr(int len)
{
	Object *obj = newobj(OSTRING); 
	obj->ptr = obj->beg = gcalloc(len + 1);
	obj->end = obj->beg + len;
	return obj; 
}
