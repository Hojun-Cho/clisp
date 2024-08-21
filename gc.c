#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define IS_MARKED(x) ((x)->type & Obj_Marked)
#define SET_MARK(x) ((x)->type |=  Obj_Marked)
#define UNSET_MARK(x) ((x)->type &= (~Obj_Marked))
#define IS_USING(x) ((x)->type & Obj_Using)
#define SET_USING(x) ((x)->type |= Obj_Using)

typedef struct
{
	int cap;
	int len;
	int total;
	int using;
	Object **arr;
}GC;

GC gc;

void*
xalloc(int sz)
{
	void *res = calloc(1, sz);
	if(res == 0)
		panic("Can't allocated %d byte", sz);
	return res;
}

static void
_init_object(Object *obj)
{
	switch(TYPE(obj)){
	case STRING:
		free(obj->beg);
		break;
    }
	memset(obj, 0, sizeof(Object)); 
	gc.using--;
}

Object*
new_object(enum Obj_Type type)
{
	gc_run();
	for(int i = 0; i < gc.len; ++i){
		for(int j = 0; j < DEFAULT_OBJS_CAP; ++j){
			Object *obj = &gc.arr[i][j];
			if(IS_USING(obj) == 0){
				gc.using++;
				obj->type = type;
				SET_USING(obj);
				return obj;
			}
		}
	}
	panic("not impl yet");
}

Object*
new_int(int val)
{
	Object *obj = new_object(INT);
	obj->value = val;
	return obj;
}

Object*
new_cons(Object *car, Object *cdr)
{
	Object *obj = new_object(CELL);
	obj->car = car;
	obj->cdr = cdr;
	return obj;
}

Object*
new_symbol(char *sym)
{
	for(Object *c = symbols; c != Nil; c = c->cdr){
		if(strcmp(sym, c->car->sym) == 0)
			return c->car;
	}
	Object *obj = new_object(SYMBOL);
	memcpy(obj->sym, sym, strlen(sym) + 1);
	symbols = new_cons(obj, symbols);
	return obj;
}

Object*
new_env(Object *vars, Object *up)
{
	Object *obj = new_object(ENV);
	obj->vars = vars;
	obj->up = up;
	return obj;
}

Object*
new_acons(Object *x, Object *y, Object *z)
{
	Object *obj = new_cons(x, y);
	return new_cons(obj, z);
}

Object*
new_primitve(Primitive fn)
{
	Object *obj = new_object(PRIM);
	obj->fn = fn;
	return obj;
}

Object*
new_function(Object *env, enum Obj_Type type, Object *params, Object *body)
{
	Object *fn = new_object(type);
	fn->params = params;
	fn->body = body;
	fn->env = env;
	return fn;
}

static Object*
_search(void *ptr)
{
	for(int i = 0; i < gc.len; ++i){
		for(int j = 0; j < DEFAULT_OBJS_CAP; ++j){
			if(&gc.arr[i][j] == ptr){
				return ptr;
			}
		}
	}
	return 0;
}

static void
_mark(Object *obj)
{
	if(obj == 0 || IS_MARKED(obj))
		return;
	SET_MARK(obj);
	switch(TYPE(obj)){
	case CELL:
		_mark(obj->car);
		_mark(obj->cdr);
		break;
	case FUNC:
		_mark(obj->params);
		_mark(obj->body);
		_mark(obj->env);
		break;
	case ENV:
		_mark(obj->vars);
		_mark(obj->up);
		break;
	}
}

static void
_gc_mark(uintptr_t bot)
{
	if(bot < (uintptr_t)stack_bot){
		panic("fuck!!");
	}
	for(uintptr_t ptr = bot;
		ptr < (uintptr_t)stack_top;
		ptr += sizeof(uintptr_t))
	{
		Object *cur = _search(*(void**)ptr);
		if(cur)
			_mark(cur);
	}
	_mark(symbols);
	_mark(root_env);
}

static void
_gc_sweep(void)
{
	for(int i = 0; i < gc.len; ++i){
		for(int j = 0; j < DEFAULT_OBJS_CAP; ++j){
			Object *obj = &gc.arr[i][j];
			if(IS_USING(obj) == 0)
				continue;
			if(IS_MARKED(obj) == 0){
				_init_object(obj);
			}else{
				UNSET_MARK(obj);
			}
		}
	}
}

void
gc_run(void)
{
	uintptr_t *ptr = (uintptr_t *)workspace;
	while(*ptr == 0)
		ptr++;
	_gc_mark((uintptr_t)ptr);
	_gc_sweep();
	printf("GC result=> total:%d, using:%d, freed:%d\n", 
		gc.total, gc.using, gc.total - gc.using);
	uintptr_t len = (uintptr_t)&ptr -(uintptr_t)ptr;
	if(len >= 32)
		memset((void*)ptr, 0, len - 32);
}

void
init_gc(void)
{
	gc.cap = 1;
	gc.len = 1;
	gc.using = 0;
	gc.total = gc.cap * DEFAULT_OBJS_CAP;
	gc.arr = xalloc(sizeof(Object *) * gc.cap);
	gc.arr[0] = xalloc(sizeof(Object) * DEFAULT_OBJS_CAP);
}
