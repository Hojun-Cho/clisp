#include "dat.h"
#include "fn.h"

static Object* evallist(Object *env, Object *list);

static int
exprlen(Object *expr)
{
	int l = 0;
	for(;expr->type==OCELL; expr=expr->cdr)
		++l;	
	return l;
}

static int
islist(Object *obj)
{
	return obj == &Nil || obj->type == OCELL;
}

static Object*
clone(Object *p)
{
	switch(p->type){
	default: panic("unreachable");
	case OSYMBOL:
	case OBLTIN: return p;
	case OINT: return newint(gc, p->num);
	case OIDENT: return newsymbol(gc, p->beg, p->ptr - p->beg);
	case OCELL: return newcons(gc, clone(p->car), clone(p->cdr));
	case OENV: return p;
	case OMACRO:
	case OFUNC: return newfn(gc, p->env, clone(p->params), clone(p->body), p->type);
	case OSTRING:{
		Object *s = newstr(gc, p->end - p->beg);
		strinit(s, p);
		return s;
		}
	}
}

static Object*
find(Object *env, Object *obj)
{
	for(Object *cur=env; cur!=&Nil; cur=cur->up)
	for(Object *p=cur->vars; p!=&Nil; p=p->cdr)
		if(strequal(obj, p->car->car))
			return clone(p->car->cdr);
	error("not exist variable");
	return 0;
}

static Object*
_newfn(Object *env, Object *l, enum OType type)
{
	if(l->type!=OCELL || islist(l->car)==0 || l->cdr->type!=OCELL)
		error("malformed function");
	for(Object *p=l->car; p->type==OCELL; p=p->cdr)
		if(p->car->type!=OIDENT)
			error("parameter is not IDNET");
	Object *params = l->car;
	Object *body = l->cdr;
	return newfn(gc, env, params, body, type);
}

static Object* 
defvar(Object *env, Object *id, Object *val)
{
	for(Object *p=env->vars; p!=&Nil; p=p->cdr)
		if(strequal(id, p->car->car))
			error("already exist variable. use setq plz...");
	return newacons(gc, id, val, env->vars);
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
	env->vars = defvar(env, l->car, macro);
	return macro;
}

static Object*
progn(Object *env, Object *list)
{
	Object *r = 0;
	for(Object *p=list; p!=&Nil; p=p->cdr){
		r = eval(env, p->car);
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
	Object *cur = env;
	Object *p = 0;
	for(; cur!=&Nil; cur=cur->up)
	for(p=cur->vars; p!=&Nil; p=p->cdr)
		if(strequal(list->car, p->car->car))
				goto found;
	error("setq not exist variable");
found:;
	return p->car->cdr = eval(env, list->cdr->car);	
}

Object*
fndefine(Object *env, Object *list)
{
	if(exprlen(list)!=2 || list->car->type!=OIDENT)
		error("Malformed define");
	Object *val = eval(env, list->cdr->car);
	env->vars = defvar(env, list->car, val);
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
	if(p->type != OCELL)
		return p;
	if(p->car == &Comma){
		if(p->cdr->type == OCELL && p->cdr->car == &Splice){
			return newcons(gc, &Splice, eval(env, p->cdr->cdr));
		}else
			return eval(env, p->cdr);
	}
	p->car = evalcomma(env, p->car);
	p->cdr = evalcomma(env, p->cdr);
	if(p->car->type == OCELL && p->car->car == &Splice){
		Object *i = p->car;
		while(i->cdr->type == OCELL && i->cdr != &Nil)
			i = i->cdr;
		if(i->type == OCELL){
			i->cdr = p->cdr;
			return p->car->cdr;
		}
		p->car = i;
	}
	return p;
}

Object*
fnbquote(Object *env, Object *list)
{
	if(exprlen(list) != 1)
		error("Malformed fnbquote");	
	return evalcomma(env, list->car);
}

Object*
fncar(Object *env, Object *list)
{
    list = evallist(env, list);
    if(list->car == &Nil)
    	return &Nil;
    if(list->car->type != OCELL)
		error("car: expected list");
    return list->car->car;
}

Object*
fncdr(Object *env, Object *list)
{
    list = evallist(env, list);
    if(list->car == &Nil)
    	return &Nil;
    if(list->car->type != OCELL)
		error("cdr: expected list");
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

Object*
fnplus(Object *env, Object *list)
{
	long sum = 0;
    Object *p=evallist(env, list);
    for(;p!=&Nil; p=p->cdr){
		if(p->car->type != OINT)
			error("+ take only number");
		sum += p->car->num;
	}
	return newint(gc, sum);
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
fnnot(Object *env, Object *list)
{
	if(list->type != OCELL)
		error("Malformed not");
	return _newint(eval(env, list->car) == &Nil);
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
		error("expected list");
	Object *car = eval(env, list->car);
	Object *cdr = evallist(env, list->cdr);
	return newcons(gc, car, cdr);
}

static Object*
enter(Object *env, Object *vars, Object *args)
{
#define cdr(x) (x!=&Nil ? x->cdr : &Nil)
#define car(x) (x!=&Nil ? x->car : &Nil)

	Object *map = &Nil;
	for(;vars->type==OCELL; vars=vars->cdr, args=cdr(args)){
		if(args != &Nil && args->type!=OCELL)
			error("Cna't apply function argment dose not match");
		Object *id  = vars->car;
		Object *val = car(args);
		map = newacons(gc, id, val, map);
	}
	if(vars != &Nil)
		map = newacons(gc, vars, args, map);
	return newenv(gc, &Nil, map, env);

#undef car
#undef cdr
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
	default:
		error("apply only tabke [MACRO BLTIN FUNC]");
    case OMACRO:
    	return applymacro(env, fn, args);
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
			return find(env, obj);
		}
	case OCELL:{
			Object *fn = eval(env, obj->car);
			if(fn == &Nil)
				return &Nil;
			Object *res = apply(env, fn, obj->cdr);
			return res;
		}
	}
}
