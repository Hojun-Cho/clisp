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
	int roots;
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
	case Obj_String:
		free(obj->beg);
		break;
    case Obj_Symbol: 
		free(obj->sym);
		break;
	case Obj_Map:
		del_map(obj);
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
	Object *obj = new_object(Obj_Int);
	SET_ATOM(obj);
	obj->value = val;
	return obj;
}

Object*
new_cons(Object *car, Object *cdr)
{
	Object *obj = new_object(Obj_Cell);
	obj->car = car;
	obj->cdr = cdr;
	return obj;
}

Object*
new_symbol(char *sym)
{
	Object *obj = map_get(symbols, sym);
	if(obj)
		return obj;
	obj = new_object(Obj_Symbol);
	SET_ATOM(obj);
	int len = strlen(sym);
	obj->sym = xalloc(len + 1);
	memmove(obj->sym, sym, len + 1);
	map_set(symbols, obj, obj); /* key and value is same */
	return obj;
}

Object*
new_env(Object *vars, Object *up)
{
	Object *obj = new_object(Obj_Env);
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
	Object *obj = new_object(Obj_Prim);
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
	if(IS_MARKED(obj))
		return;
	SET_MARK(obj);
	switch(TYPE(obj)){
	case Obj_Cell:
		_mark(obj->car);
		_mark(obj->cdr);
		break;
	case Obj_Func:
		_mark(obj->params);
		_mark(obj->body);
		_mark(obj->env);
		break;
	case Obj_Env:
		_mark(obj->vars);
		_mark(obj->up);
		break;
	case Obj_Map:
		map_iterate(obj, _mark);
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
entry_root(Object *obj)
{
 	uintptr_t *ptr = (uintptr_t *)stack_top;
	ptr -= 1;
	ptr[-gc.roots] = (uintptr_t)obj;
	gc.roots++;
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
	gc.roots = 0;
	gc.total = gc.cap * DEFAULT_OBJS_CAP;
	gc.arr = xalloc(sizeof(Object *) * gc.cap);
	gc.arr[0] = xalloc(sizeof(Object) * DEFAULT_OBJS_CAP);
}
