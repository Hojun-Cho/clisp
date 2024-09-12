#include "dat.h"
#include "fn.h"
#include <assert.h>

#define cdr(x) ((x)!= &Nil && (x)->type == OCELL ? (x)->cdr : &Nil)
#define car(x) ((x)!= &Nil && (x)->type == OCELL ? (x)->car : &Nil)

static char *typtab[] = {
	[ONONE] = "error",
	[OBLTIN] = "bltin",
	[OSYMBOL] = "symbol",
	[OCELL] = "cell",
	[OIDENT] = "ident",
	[OSTRING] = "string",
	[OINT] = "int",
	[OFUNC] = "func",
	[OMACRO] = "macro",
	[OENV] = "env",
};

static Object* evallist(Object *env, Object *list);

static int
exprlen(Object *expr)
{
	int l = 0;
	while(expr != &Nil){
		expr = cdr(expr);
		l += 1;
	}
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
	case OFRAME:
	case OENV:
	case OSYMBOL:
	case OINT:
	case OIDENT:
	case OBLTIN:
		return p;
	case OMACRO:
	case OFUNC:
		return newfn(gc, p->frame, clone(p->params), clone(p->body), p->type);
	case OCELL:
		return newcons(gc, clone(p->car), clone(p->cdr));
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
	for(Object *cur=env->sp->car; cur!=&Nil; cur=cur->up)
	for(Object *p=cur->local; p!=&Nil; p=cdr(p))
		if(strequal(obj, car(car(p))))
			return clone(cdr(car(p)));
	error("not exist variable");
	return 0;
}

static Object*
_newfn(Object *env, Object *l, enum OType type)
{
	if(l->type!=OCELL || islist(l->car)==0 || l->cdr->type!=OCELL)
		error("malformed function");
	for(Object *p=l->car; p->type==OCELL; p=cdr(p))
		if(p->car->type!=OIDENT)
			error("parameter is not IDNET");
	Object *params = l->car;
	Object *body = l->cdr;
	return newfn(gc, env->sp->car, params, body, type);
}

static void 
defvar(Object *env, Object *id, Object *val)
{
	if(id->type != OIDENT)
		error("can't define, already using id");
	Object *frame = env->bp->car;
	for(Object *p=frame->local; p!=&Nil; p=cdr(p))
		if(strequal(id, car(car(p))))
			error("already exist variable. use setq plz...");
	frame->local = newacons(gc, id, val, frame->local);
}

Object*
fnlambda(Object *env, Object *l)
{
	return _newfn(env, l, OFUNC);
}

Object*
fndefmacro(Object *env, Object *l)
{
	if(l->type != OCELL)
		error("Malformed macro");
	Object *macro = _newfn(env, l->cdr, OMACRO);
	defvar(env, l->car, macro);
	return macro;
}

static Object*
progn(Object *env, Object *list)
{
	Object *r = &Nil;
	for(Object *p=list; p!=&Nil; p=cdr(p)){
		r = eval(env, car(p));
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
	if(list->type != OCELL || exprlen(list)!=2 || list->car->type!=OIDENT)
		error("Malformed setq");
	for(Object *frame=env->sp->car; frame!=&Nil; frame=frame->up)
	for(Object *p=frame->local; p!=&Nil; p=cdr(p))
		if(strequal(list->car, car(car(p))))
			return p->car->cdr = eval(env, car(cdr(list)));

	error("setq not exist variable");
	return 0;
}

static void
enter(Object *env, Object *tag, Object *local, Object *up)
{
	assert(env->bp != &Nil);
	Object *frame = newframe(gc, tag, local, up);
	env->sp = env->sp->cdr = newcons(gc, frame, &Nil);
}

static void
leave(Object *env)
{
	assert(env->sp != env->bp);
	Object *p = env->bp;
	while(cdr(p) != env->sp)
		p = p->cdr; 
	p->cdr = &Nil;
	env->sp = p;
}

Object*
fnlet(Object *env, Object *list)
{
	if(exprlen(list) < 2)
		error("let (vars) bodys");
	Object *local = &Nil;
	for(Object *p=car(list); p!=&Nil; p=cdr(p)){
		Object *id = car(car(p));
		Object *val = eval(env, car(cdr(car(p))));
		local = newacons(gc, id, val, local);
	}
	enter(env, &Let, local, env->sp->car);
	Object *res = progn(env, cdr(list));
	leave(env);
	return res;
}

Object*
fndefine(Object *env, Object *list)
{
	if(exprlen(list)!=2)
		error("Malformed define");
	Object *val = eval(env, car(cdr(list)));
	defvar(env, car(list), val);
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
		if(car(cdr(p)) == &Splice)
			return newcons(gc, &Splice, eval(env, p->cdr->cdr));
		return eval(env, p->cdr);
	}
	p->car = evalcomma(env, p->car);
	p->cdr = evalcomma(env, p->cdr);
	if(car(car(p)) == &Splice){
		Object *i = p->car;
		while(cdr(i) != &Nil)
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
	return evalcomma(env, car(list));
}

Object*
fncar(Object *env, Object *list)
{
    list = evallist(env, list);
	if(car(list)->type != OCELL)
		error("expected list");
	return car(car(list));
}

Object*
fncdr(Object *env, Object *list)
{
    list = evallist(env, list);
	if(car(list)->type != OCELL)
		error("expected list");
	return cdr(car(list));
}

Object*
fncons(Object *env, Object *list)
{
    if(exprlen(list) != 2)
        error("Malformoed cons");
    list = evallist(env, list);
    if(list->type != OCELL)
    	error("cons:bad list");
    list->cdr = car(list->cdr);
    return list;
}

Object*
fnplus(Object *env, Object *list)
{
	long sum = 0;
    Object *p=evallist(env, list);
    for(;p!=&Nil; p=cdr(p)){
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
    if(car(p)->type != OINT)
		error("* take only [INT]");
	long sum = p->car->num;
    for(p=p->cdr;p!=&Nil; p=cdr(p)){
		if(car(p)->type != OINT)
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
    for(p=p->cdr;p!=&Nil; p=cdr(p)){
		if(car(p)->type != OINT)
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
    for(p=p->cdr;p!=&Nil; p=cdr(p)){
		if(car(p)->type != OINT)
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
    Object *a = eval(env, car(list));
    Object *b = eval(env, car(cdr(list)));
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
	if(exprlen(list) != 1)
		error("Malformed not");
	return _newint(eval(env, car(list)) == &Nil);
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
	if(cdr(list)->type != OCELL)
		error("Malformed if stmt");
	Object *test = list->car;
	Object *then = car(cdr(list));
	Object *else_ = car(cdr(cdr(list)));
	if(eval(env, test)!=&Nil)
		return eval(env, then);
	return eval(env, else_);
}

static Object*
evallist(Object *env, Object *list)
{
	if(list == &Nil)
		return &Nil;
	Object *car = eval(env, car(list));
	Object *cdr = evallist(env, cdr(list));
	return newcons(gc, car, cdr);
}

static Object*
applyargs(Object *fn, Object *args)
{
	Object *map = &Nil;
	Object *vars = fn->params;
	for(;vars->type==OCELL; vars=cdr(vars), args=cdr(args)){
		if(args != &Nil && args->type!=OCELL)
			error("Cna't apply function argment dose not match");
		Object *id  = car(vars);
		Object *val = car(args);
		map = newacons(gc, id, val, map);
	}
	if(vars != &Nil)
		map = newacons(gc, vars, args, map);
	return map;
}

static Object*
applyfn(Object *env, Object *tag, Object *fn, Object *args)
{
	Object *local = applyargs(fn, args);
	enter(env, tag, local,fn->frame);
	Object *res = progn(env, fn->body);
	leave(env);
	return res;
}

static Object*
applymacro(Object *env, Object *tag, Object* fn, Object *args)
{
	Object *local = applyargs(fn, args);
	enter(env, tag, local, fn->frame);
	Object *r = progn(env, fn->body);
	leave(env);
	return eval(env, r);
}

static Object*
apply(Object *env, Object *tag, Object *fn, Object *args)
{
	switch(fn->type){
	default:
		error("apply:can't eval type");
		return 0;
	case OBLTIN:{
            Bltinfn blt = bltinlookup(fn);
            return blt(env, args);
        }
	case OMACRO:
		return applymacro(env, tag, fn, args);
	case OFUNC:{
			Object *elist = evallist(env, args);
			Object*res = applyfn(env, tag, fn, elist);
			return res;
		}
	}
}

Object*
eval(Object *env, Object *obj)
{
	switch(obj->type){
	default:
		error("eval: can't eval type");
		return 0;
	case OSTRING:
	case OINT:
	case OBLTIN:
	case OSYMBOL:
		return obj;
	case OIDENT:
			return find(env, obj);
	case OCELL:{
			Object *fn = eval(env, obj->car);
			Object *res = apply(env, obj->car, fn, obj->cdr);
			return res;
		}
	}
}
