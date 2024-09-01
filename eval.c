#include "dat.h"
#include "fn.h"

static Object* evallist(Object *env, Object *list);

static int
exprlen(Object *expr)
{
	int l = 0;
	for(;expr->type==OCELL; expr=expr->cdr)
		++l;	
	if(expr != &Nil)
		error("Not list type");
	return l;
}

static int
islist(Object *obj)
{
	return obj == &Nil || obj->type == OCELL;
}

static Object*
find(Object *env, Object *obj)
{
	for(Object *cur=env; cur!=&Nil; cur=cur->up){
		for(Object *p=cur->vars; p!=&Nil; p=p->cdr)
			if(strequal(obj, p->car->car))
				return p->car;
	}
	return 0;
}

static Object*
enter(Object *env, Object *vars, Object *args)
{
	Object *map = &Nil;
	for(;vars->type==OCELL; vars=vars->cdr,args=args->cdr){
		if(args->type!=OCELL)
			error("Cna't apply function argment dose not match");
		Object *id  = vars->car;
		Object *val = args->car;
		map = newacons(gc, id, val, map);
	}
	return newenv(gc, &Nil, map, env);
}

Object*
fnlambda(Object *env, Object *l)
{
	if(l->type!=OCELL || islist(l->car)==0 || l->cdr->type!=OCELL)
		error("malformed function");
	for(Object *p=l->car; p->type==OCELL; p=p->cdr){
		if(p->car->type!=OIDENT)
			error("parameter is not IDNET");
	}
	Object *params = l->car;
	Object *body = l->cdr;
	return newfn(gc, env, params, body);
}

Object*
fnsetq(Object *env, Object *list)
{
	if(exprlen(list)!=2 || list->car->type!=OIDENT)
		error("Malformed setq");
	Object *obj = find(env, list->car);
	if(obj == 0)
		error("Not exist variable");
	return obj->cdr = eval(env, list->cdr->car);
}

Object*
fndefine(Object *env, Object *list)
{
	if(exprlen(list)!=2 || list->car->type!=OIDENT)
		error("Malformed define");
	Object *val = eval(env, list->cdr->car);
	Object *obj = find(env, list->car);
	if(obj)
		return obj->cdr = val;
	return env->vars = newacons(gc, list->car, val, env->vars);
}

Object*
fnplus(Object *env, Object *list)
{
	long sum = 0;
	for(Object *p=evallist(env, list); p!=&Nil; p=p->cdr){
		if(p->car->type != OINT)
			error("+ take only number");
		sum += p->car->num;
	}
	return newint(gc, sum);
}

static Object*
evallist(Object *env, Object *list)
{
	if(list == &Nil)
		return &Nil;
	if(list->type != OCELL)
		error("type is not list");
	Object *car = eval(env, list->car);
	Object *cdr = evallist(env, list->cdr);
	return newcons(gc, car, cdr);
}

static Object*
applyfn(Object *fn, Object *args)
{
	Object *env = enter(fn->env, fn->params, args);
	Object *r = 0;
	for(Object *p=fn->body; p!=&Nil; p=p->cdr){
		r = p->car;
		r = eval(env, r);
	}
	return r;
}

static Object*
apply(Object *env, Object *fn, Object *args)
{
	if(islist(args) == 0)
		error("args is not list type");
	switch(fn->type){
	case OBLTIN:
		Bltinfn blt = bltinlookup(fn);
		if(blt==0)
			error("not builtin type!");
		return blt(env, args);
	case OFUNC:{
			Object *elist = evallist(env, args);
			Object*res = applyfn(fn, elist);
			return res;
		}
	}
	error("can't apply");
	return 0;
}

Object*
eval(Object *env, Object *obj)
{
	switch(obj->type){
	case OINT:
	case OSTRING:
	case OBLTIN:
	case OSYMBOL:
		return obj;
	case OIDENT:{
			Object* val = find(env, obj);
			if(val == 0)
				error("not exist '%s'", obj->beg);
			return val->cdr;
		}
	case OCELL:{
			Object *fn = eval(env, obj->car);
			if(fn->type!=OFUNC&&fn->type!=OBLTIN)
				error("expected function type");
			Object *res = apply(env, fn, obj->cdr);
			return res;
		}
	}
	error("can't apply");
	return 0;
}
