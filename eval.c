#include "dat.h"
#include "fn.h"

static int _is_list(Object *obj);
static Object* _eval_list(Object *env, Object *list);

static Object*
_entry_fn(Object *env, Object *list, enum Obj_Type type)
{
	if(TYPE(list)!= Obj_Cell ||
		_is_list(list->car) == 0 ||
		TYPE(list->cdr) != Obj_Cell)
		error_expr("malformed lambda", list);
	Object *ptr = list->car;
	for(; TYPE(ptr) == Obj_Cell; ptr = ptr->cdr){
		if(TYPE(ptr->car) != Obj_Symbol)
			error_expr("parameter is not Obj_Symbol", ptr);
	}
	Object *params = list->car;
	Object *body = list->cdr;
	return new_function(env, type, params, body);
}

Object*
fn_lambda(Object *env, Object *list)
{	
	return _entry_fn(env, list, Obj_Func);
}

Object*
fn_plus(Object *env, Object *list)
{
	int sum = 0;
	for(Object *p = _eval_list(env, list); p != Nil; p = p->cdr){
		if(TYPE(p->car) != Obj_Int)
			error_expr("+ take only numbers", p);
		sum += p->car->value;
	}
	return new_int(sum);
}

Object*
fn_minus(Object *env, Object *list)
{
	Object *p = _eval_list(env, list);
	if(TYPE(p->car) != Obj_Int)
		error_expr("- take only numbers", p);
	int sum = -p->car->value;
	p = p->cdr;
	for(; p != Nil; p = p->cdr){
		if(TYPE(p->car) != Obj_Int)
			error_expr("- take only numbers", p);
		sum += p->car->value;
	}
	return new_int(sum);
}

static Object*
_find(Object *env, Object *sym)
{
	for(Object *ptr = env; ptr != Nil; ptr = ptr->up){
		Object *res = map_get(ptr->vars, sym);
		if(res)
			return res;
	}
	return 0;
}

static Object*
_progn(Object *env, Object *list)
{
	Object *r = 0;
	for(Object *lp = list; lp != Nil; lp = lp->cdr){
		r = lp->car;
		r = eval(env, r);
	}
	return r;
}

static int
_is_list(Object *obj)
{
	return obj == Nil || TYPE(obj) == Obj_Cell;
}

static Object*
_eval_list(Object *env, Object *list)
{
	Object *head = Nil;
	for(Object *lp = list; lp != Nil; lp = lp->cdr){
		Object *expr = lp->car;
		Object *res = eval(env, expr);
		head = new_cons(res, head);
	}
	return reverse(head);
}

static Object*
_apply_func(Object *env, Object *fn, Object *args)
{
	Object *params = fn->params;
	Object *nenv = fn->env;
	nenv = push_env(nenv, params, args);
	Object *body = fn->body;
	return _progn(nenv, body);
}

static Object*
_apply(Object *env, Object *fn, Object *args)
{
	if(_is_list(args) == 0)
		error_expr("argements is not list", args);
	switch(TYPE(fn)){
	case Obj_Prim:
		return fn->fn(env, args);
	case Obj_Func:{
			Object *elist = _eval_list(env, args);
			return _apply_func(env, fn, elist);
		}
	}
	error_expr("can't evaulate Expr", fn);
}

Object*
eval(Object *env, Object *obj)
{
	switch(TYPE(obj)){
	case Obj_Int:
	case Obj_Prim:
	case Obj_Func:
	case Obj_True:
	case Obj_Nil:
	case Obj_String:
		return obj;
	case Obj_Symbol:{
			Object *bind = _find(env, obj);
			if(bind == 0)
				error("undefined symbol %s", obj->sym);
			return bind;
		}
	case Obj_Cell:{
			Object *fn = obj->car;
			fn = eval(env, fn);
			Object *args = obj->cdr;
			if(TYPE(fn) != Obj_Prim && TYPE(fn) != Obj_Func)
				error_expr("Cell head is not function", fn);
			return _apply(env, fn, args);
		}
	}
	error_expr("can't evaulate Expr", obj);
}
