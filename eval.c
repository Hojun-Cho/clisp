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
_newfn(Object *env, Object *l, enum OType type)
{
	if(l->type!=OCELL || islist(l->car)==0 || l->cdr->type!=OCELL)
		error("malformed function");
	for(Object *p=l->car; p->type==OCELL; p=p->cdr){
		if(p->car->type!=OIDENT)
			error("parameter is not IDNET");
	}
	Object *params = l->car;
	Object *body = l->cdr;
	return newfn(gc, env, params, body, type);
}

static void
setvar(Object *env, Object *id, Object *val)
{
	Object *obj = find(env, id);
	if(obj == 0)
		env->vars = newacons(gc, id, val, env->vars);
	else
		obj->cdr = val;
}

Object*
fndefn(Object *env, Object *list)
{
	Object *fn = _newfn(env, list->cdr, OFUNC);
	setvar(env, list->car, fn);
	return fn;
}

Object*
fnlambda(Object *env, Object *l)
{
	return _newfn(env, l, OFUNC);
}

Object*
fnmacro(Object *env, Object *l)
{
	Object *macro = _newfn(env, l->cdr, OMACRO);
	setvar(env, l->car, macro);
	return macro;
}

static Object*
progn(Object *env, Object *list)
{
	Object *r = 0;
	for(Object *p=list; p!=&Nil; p=p->cdr){
		r = p->car;
		r = eval(env, r);
	}
	return r;
}

Object*
fnprogn(Object *env, Object *list)
{
	return progn(env, list);
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
	setvar(env, list->car, val);
	return val;
}

Object*
fnquote(Object *env, Object *list)
{
	if(exprlen(list)!=1)
		error("Malformed quote");
    return list->car;
}

static Object*
evalcomma(Object *env, Object *p)
{
	enum { VISITED = 1 << 9 };

	if(p->type == OCELL){
		if(p->flag & VISITED)
			return p;
		p->flag |= VISITED;
		if(p->car == &Comma){
			p = eval(env, p->cdr);
			p->flag |= VISITED;
			return p;
		}
		p->car = evalcomma(env, p->car);
		p->cdr = evalcomma(env, p->cdr);
	}
	return p;
}

Object*
fnbquote(Object *env, Object *list)
{
	if(list->cdr != &Nil){
		printexpr(list);
		error("fnbquote expected cdr is nil");	
	}
	list = evalcomma(env, list->car);
	return list;
}

Object*
fncar(Object *env, Object *list)
{
    list = evallist(env, list);
    if(list->car->type != OCELL || list->cdr != &Nil)
		error("Malformed Car");
    return list->car->car;
}

Object*
fncdr(Object *env, Object *list)
{
    list = evallist(env, list);
    if(list->car->type != OCELL || list->cdr != &Nil)
		error("Malformed Car");
    return list->car->cdr;
}

Object*
fncons(Object *env, Object *list)
{
    if(exprlen(list) != 2)
        error("Malformoed cons");
    list = evallist(env, list);
    list->cdr = list->cdr->car;
    return list;
}

static Object*
plusint(Object *env, Object *p)
{
	long sum = 0;
    for(;p!=&Nil; p=p->cdr){
		if(p->car->type != OINT)
			error("+ take only number");
		sum += p->car->num;
	}
	return newint(gc, sum);
}

static Object*
plusstr(Object *env, Object *p)
{
    Object *str = newstr(gc, 16);
    for(;p!=&Nil; p=p->cdr){
		if(p->car->type != OSTRING)
			error("+ take only number");
        str = strputs(str, p->car);
	}
    return str;
}

Object*
fnplus(Object *env, Object *list)
{
    Object *p=evallist(env, list);
    switch(p->car->type){
    default: error("+ take only [STRING, INT]");
    case OSTRING: return plusstr(env ,p);
    case OINT: return plusint(env, p);
    }
}

Object*
fnmul(Object *env, Object *list)
{
    Object *p = evallist(env, list);
    if(p->car->type != OINT)
		error("* take only [INT]");
	long sum = p->car->num;
    for(p=p->cdr;p!=&Nil; p=p->cdr){
		if(p->car->type != OINT)
			error("* take only [INT]");
		sum *= p->car->num;
	}
	return newint(gc, sum);
}

Object*
fndiv(Object *env, Object *list)
{
    Object *p=evallist(env, list);
    if(p->car->type != OINT)
		error("/ take only [INT]");
	long sum = p->car->num;
    for(p=p->cdr;p!=&Nil; p=p->cdr){
		if(p->car->type != OINT)
			error("/ take only [INT]");
        if(p->car->num == 0)
            error("Can't div zero");
		sum /= p->car->num;
	}
	return newint(gc, sum);
}

Object*
fnmod(Object *env, Object *list)
{
    Object *p=evallist(env, list);
	if(p->car->type != OINT)
		error("%% take only [INT]");
	long sum = p->car->num;
    for(p=p->cdr;p!=&Nil; p=p->cdr){
		if(p->car->type != OINT)
			error("%% take only [INT]");
        if(p->car->num == 0)
            error("Can't mod zero");
		sum %= p->car->num;
	}
	return newint(gc, sum);
}

static long
cmp(Object *env, Object *list)
{
    Object *a = eval(env, list->car);
    Object *b = eval(env, list->cdr->car);
	if(a->type != OINT || b->type != OINT)
		error("cmp only take [INT]");
	return a->num - b->num;	
}

static Object*
_newint(int n)
{
	if(n == 0)
		return &Nil;
	return newint(gc, 1);
}

Object*
fneq(Object *env, Object *list)
{
	return _newint(cmp(env, list) == 0);
}

Object*
fnge(Object *env, Object *list)
{
	return _newint(cmp(env, list) >= 0);
}

Object*
fngt(Object *env, Object *list)
{
	return _newint(cmp(env, list) > 0);
}

Object*
fnle(Object *env, Object *list)
{
	return _newint(cmp(env, list) <= 0);
}

Object*
fnlt(Object *env, Object *list)
{
	return _newint(cmp(env, list) < 0);
}

Object*
fnne(Object *env, Object *list)
{
	return _newint(cmp(env, list) != 0);
}

Object*
fnif(Object *env, Object *list)
{
	if(list->type != OCELL || list->cdr->type != OCELL)
		error("Malformed if stmt");
	if(eval(env, list->car)!=&Nil)
		return eval(env, list->cdr->car);
	if(list->cdr->cdr == &Nil)
		return &Nil;
	if(list->cdr->cdr->type != OCELL)
		error("Malformed else stmt");
	return eval(env, list->cdr->cdr->car);
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
	if(vars != &Nil)
		map = newacons(gc, vars, args, map);
	return newenv(gc, &Nil, map, env);
}

static Object*
applyfn(Object *fn, Object *args)
{
	Object *env = enter(fn->env, fn->params, args);
	return progn(env, fn->body);
}

static Object*
applymacro(Object *env, Object* fn, Object *args)
{
	Object *nenv = enter(fn->env, fn->params, args);
	Object *r = 0;
	for(Object *p=fn->body; p!=&Nil; p=p->cdr){
		r = p->car;
		r = eval(nenv, r);
	}
	return eval(env, r);
}

static Object*
apply(Object *env, Object *fn, Object *args)
{
	if(islist(args) == 0)
		error("args is not list type");
	switch(fn->type){
	default: error("can't apply");
	case OBLTIN:{
            Bltinfn blt = bltinlookup(fn);
            if(blt==0)
                error("not builtin type!");
            return blt(env, args);
        }
	case OFUNC:{
			Object *elist = evallist(env, args);
			Object*res = applyfn(fn, elist);
			return res;
		}
	}
}

Object*
eval(Object *env, Object *obj)
{
	switch(obj->type){
	default:
		error("can't eval");
	case OSTRING:
	case OINT:
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
			if(fn->type == OMACRO)
				return applymacro(env, fn, obj->cdr);
			if(fn->type!=OFUNC&&fn->type!=OBLTIN)
				error("expected function type");
			Object *res = apply(env, fn, obj->cdr);
			return res;
		}
	}
}
