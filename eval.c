#include "dat.h"
#include "fn.h"

static Object** _eval_list(Object **env, Object **list);

static int
_is_list(Object **obj)
{
	return obj == Nil || TYPE(*obj) == CELL;
}

static int
_length(Object **list)
{
	if(list == Nil)
		return 0;
	if(TYPE(*list) != CELL)
		error_expr("expected list", list);
	return _length((*list)->cdr) + 1;
}

static Object**
_entry_fn(Object **env, Object **list, enum Obj_Type type)
{
	if(TYPE(*list)!=CELL || _is_list((*list)->car)==0 || TYPE(*(*list)->cdr)!=CELL)
		error_expr("malformed lambda", list);
	for(Object **p=(*list)->car; TYPE(*p)==CELL; p=(*p)->cdr){
		if(TYPE(*(*p)->car) != SYMBOL)
			error_expr("parameter is not SYMBOL", p);
	}
	Object **params = (*list)->car;
	Object **body = (*list)->cdr;
	return new_function(env, type, params, body);
}

static Object**
_lambda(Object **env, Object **list)
{	
	return _entry_fn(env, list, FUNC);
}

static Object**
_plus(Object **env, Object **list)
{
	long sum = 0;
	for(Object **p=_eval_list(env, list); p!=Nil; p=(*p)->cdr){
		if(TYPE(*(*p)->car) != INT)
			error_expr("+ take only numbers", p);
		sum += (*((*p)->car))->value;
	}
	return new_int(sum);
}

static Object**
_minus(Object **env, Object **list)
{
	Object **p = _eval_list(env, list);
	if(TYPE(*((*p)->car)) != INT)
		error_expr("- take only numbers", p);
	int sum = -(*(*p)->car)->value;
	p = (*p)->cdr;
	for(; p != Nil; p = (*p)->cdr){
		if(TYPE(*((*p)->car)) != INT)
			error_expr("- take only numbers", p);
		sum += (*(*p)->car)->value;
	}
	return new_int(sum);
}

static Object**
_cons(Object **env, Object **list)
{
	int len = _length(list);
	if(len != 2)
		error("Malformed cons expected length is 2, but %d", len);
	Object **obj = _eval_list(env, list);
	(*obj)->cdr = (*((*obj)->cdr))->car;
	return obj;
}

static Object**
_define(Object **env, Object **list)
{
	if(_length(list) != 2 || TYPE(*(*list)->car) != SYMBOL)
		error_expr("Malformed define", list);
	Object **sym = (*list)->car;
	Object **val = eval(env, (*(*list)->cdr)->car);
	add_variable(sym, val, env);
	return val;
}

static Object**
_quote(Object **env, Object **list)
{
	if(_length(list) != 1)
		error_expr("Malformed quote", list);
	return (*list)->car;
}

static Object**
_car(Object **env, Object **list)
{
	Object **args = _eval_list(env, list);
	if(TYPE(*(*args)->car) != CELL || (*args)->cdr != Nil)
		error_expr("Malformed car", list);
	return (*(*args)->car)->car;
}

static Object**
_cdr(Object **env, Object **list)
{
	Object **args = _eval_list(env, list);
	if(TYPE(*(*args)->car) != CELL)
		error_expr("Malformed cdr", list);
	return (*(*args)->car)->cdr;
}

Object**
_find(Object **env, Object **sym)
{
	for(Object **p=env; p!=Nil; p=(*p)->up){
		for(Object **c = (*p)->vars; c!=Nil; c=(*c)->cdr){
			if(sym == (*(*c)->car)->car)
				return (*c)->car;
		}
	}
	return 0;
}

static Object**
_progn(Object **env, Object **list)
{
	Object **r = 0;
	for(Object **lp=list; lp!=Nil; lp=(*lp)->cdr){
		r = (*lp)->car;
		r = eval(env, r);
	}
	return r;
}

static Object**
_eval_list(Object **env, Object **list)
{
	if(list == Nil)
		return Nil;
	if(TYPE(*list) != CELL)
		error_expr("type is not list", list);
	Object **car = eval(env, (*list)->car);
	Object **cdr = _eval_list(env, (*list)->cdr);
	return new_cons(car, cdr);
}

static Object**
_apply_func(Object **env, Object **fn, Object **args)
{
	Object **params = (*fn)->params;
	Object **nenv = (*fn)->env;
	nenv = push_env(nenv, params, args);
	Object **body = (*fn)->body;
	return _progn(nenv, body);
}

static Object**
_apply(Object **env, Object **fn, Object **args)
{
	if(_is_list(args) == 0)
		error_expr("argements is not list", args);
	switch(TYPE(*fn)){
	case PRIM:
		return (*fn)->fn(env, args);
	case FUNC:{
			Object **elist = _eval_list(env, args);
			return _apply_func(env, fn, elist);
		}
	}
	error_expr("can't evaulate Expr", fn);
}

Object**
eval(Object **env, Object **obj)
{
	switch(TYPE(*obj)){
	case INT:
	case PRIM:
	case FUNC:
		return obj;
	case SYMBOL:{
			Object **bind = _find(env, obj);
			if(bind == 0)
				error("undefined symbol %s", (*obj)->sym);
			return (*bind)->cdr;
		}
	case CELL:{
			Object **fn = (*obj)->car;
			fn = eval(env, fn);
			if(TYPE(*fn) != PRIM && TYPE(*fn) != FUNC)
				error_expr("expected function", fn);
			Object **args = (*obj)->cdr;
			return _apply(env, fn, args);
		}
	}
	error_expr("can't evaulate Expr", obj);
}

void
init_primitive(void)
{
	add_primitive(Plus, _plus, root_env);
	add_primitive(Minus, _minus, root_env);
	add_primitive(Lambda, _lambda, root_env);
	add_primitive(Car, _car, root_env);
	add_primitive(Cdr, _cdr, root_env);
	add_primitive(Quote, _quote, root_env);
    add_primitive(Cons, _cons, root_env);
	add_primitive(Define, _define, root_env);
	/*
     *add_primitive("setq", fn_setq, root_env);
     *add_primitive("setcar", fn_setcar, root_env);
     *add_primitive("while", fn_while, root_env);
     *add_primitive("gensym", fn_gensym, root_env);
	 *add_primitive("<", fn_lt, root_env);
     *add_primitive("defun", fn_defun, root_env);
     *add_primitive("defmacro", fn_defmacro, root_env);
     *add_primitive("macroexpand", fn_macroexpand, root_env);
     *add_primitive("if", fn_if, root_env);
     *add_primitive("=", fn_num_eq, root_env);
     *add_primitive("eq", fn_eq, root_env);
     *add_primitive("println", fn_println, root_env);
	 */
}
