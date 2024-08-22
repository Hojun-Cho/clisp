#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#define ALIGN (sizeof(int) * 2)
#define OBJ_SIZE ALIGN
#define DEFAULT_OBJ_SLOTS 64
#define IS_MARKED(x) ((x)->type & Obj_Marked)
#define SET_MARK(x) ((x)->type |=  Obj_Marked)
#define UNSET_MARK(x) ((x)->type &= (~Obj_Marked))

typedef struct OList OList;
struct OList
{
	Object *arr[DEFAULT_OBJ_SLOTS];
	OList *next;
};

typedef struct
{
	uintptr_t beg;
	uintptr_t end;
	int total;
	int using;
	OList *objs;
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

static int
_alignment(int x)
{
	if(x % ALIGN)
		x += ALIGN - x % ALIGN;
	return x;
}

static Object**
_entry(void *obj)
{
	OList *last = 0;
	OList *p = gc.objs;
	while(p){
		for(int i = 0; i<DEFAULT_OBJ_SLOTS; ++i){
			if(p->arr[i] == 0){
				p->arr[i] = obj;
				return &p->arr[i];
			}
		}
		last = p;
		p = p->next;
	}
	p = last->next = xalloc(sizeof(OList));
	p->arr[0] = obj;
	return &p->arr[0];
}

static uintptr_t
_next(uintptr_t ptr)
{
	while(ptr < gc.end){
		Object *obj = (Object*)ptr;
		if(obj->size)
			return ptr;
		ptr += ALIGN;
	}
	return 0;
}

static Object* 
_new(int size, enum Obj_Type type)
{
	size = _alignment(size + OBJ_SIZE);
	assert(size < gc.total);

	uintptr_t cur = gc.beg;
	while(1){
		uintptr_t res = _next(cur);
		if(res == 0 || res - cur >= (uintptr_t)size)
			break;
		cur = res + *(int*)res;
	}
	gc.using += size;
	Object *obj = (Object*)cur;
	obj->size = size;
	obj->type = type;
	return obj;
}

Object**
new_int(long val)
{
	Object *obj = _new(sizeof(long), INT);
	obj->value = val;
	return _entry(obj);
}

Object**
new_cons(Object **car, Object **cdr)
{
	Object *obj = _new(sizeof(void*)*2, CELL);
	obj->car = car;
	obj->cdr = cdr;
	return _entry(obj);
}

Object**
new_env(Object **vars, Object **up)
{
	Object *obj = _new(sizeof(void*)*2, ENV);
	obj->vars = vars;
	obj->up = up;
	return _entry(obj);
}

Object**
new_acons(Object **x, Object **y, Object **z)
{
	Object **obj = new_cons(x, y);
	return new_cons(obj, z);
}

Object**
new_primitve(Primitive fn)
{
	Object *obj = _new(sizeof(void*), PRIM);
	obj->fn = fn;
	return _entry(obj);
}

Object**
new_function(Object **env, enum Obj_Type type, Object **params, Object **body)
{
	Object *fn = _new(sizeof(void*)*3, type);
	fn->params = params;
	fn->body = body;
	fn->env = env;
	return _entry(fn);
}

static void
_set_ptr(Object *obj, void *dst)
{
	uintptr_t ptr = (uintptr_t)obj;
	ptr += OBJ_SIZE;
	ptr += sizeof(ptr);
	memcpy(dst, &ptr, sizeof(ptr));
}

Object**
new_symbol(char *sym)
{
	for(Object **c = symbols; c != Nil; c = (*c)->cdr){
		if(strcmp(sym, (*((*c)->car))->sym) == 0)
			return (*c)->car;
	}
	int len = strlen(sym);
	Object *obj = _new(sizeof(void*) + len + 1, SYMBOL);
	_set_ptr(obj, &obj->sym);
	memcpy(obj->sym, sym, len + 1);
	Object **oobj = _entry(obj);
	symbols = new_cons(oobj, symbols);
	return oobj;
}

static void
_init_object(Object *obj)
{
	printf("sweep %p %d byte type:%d\n", obj, obj->size, TYPE(obj));
	gc.using -= obj->size;
	memset(obj, 0, obj->size);
}

static void
_gc_sweep(void)
{
	OList *p = gc.objs;
	while(p){
		for(int i=0; i<DEFAULT_OBJ_SLOTS; ++i){
			Object *obj = p->arr[i];
			if(obj == 0)
				continue;
			if(IS_MARKED(obj)==0){
				_init_object(obj);
				p->arr[i] = 0;
			}else{
				UNSET_MARK(obj);
			}
		}
		p = p->next;
	}
}

static void
_mark(Object *obj)
{
	if(obj == 0 || IS_MARKED(obj))
		return;
	SET_MARK(obj);
	switch(TYPE(obj)){
	case CELL:
		_mark(*obj->car);
		_mark(*obj->cdr);
		break;
	case FUNC:
		_mark(*obj->params);
		_mark(*obj->body);
		_mark(*obj->env);
		break;
	case ENV:
		_mark(*obj->vars);
		_mark(*obj->up);
		break;
	}
}

static Object*
_find(uintptr_t addr)
{
	OList *p = gc.objs;
	while(p){
		if((uintptr_t)&p->arr[0]<=addr && addr<(uintptr_t)&p->arr[DEFAULT_OBJ_SLOTS])
			return *(Object**)addr;	
		p = p->next;
	}
	if(gc.beg <= addr && addr<gc.end){
		uintptr_t cur = gc.beg;
		while(1){
			cur = _next(cur);
			if(cur == 0)
				break;
			Object *obj = (Object*)cur;
			if(cur <= addr && addr < cur + obj->size)
				return (Object*)obj;
			cur += obj->size;
		}
	}
	return 0;
}

static void
_gc_mark(uintptr_t bot)
{
	if(bot < (uintptr_t)stack_bot){
		panic("out range");
	}
	for(uintptr_t ptr = bot;
		ptr < (uintptr_t)stack_top;
		ptr += sizeof(uintptr_t))
	{
		Object *cur = _find((uintptr_t)*(void**)ptr);
		if(cur)
			_mark(cur);
	}
	_mark(*root_env);
	_mark(*symbols);
}

void
gc_run(void)
{
	psetjmp(root_stack); /* push current register */
	printf("GC before=> total:%d, using:%d, remain:%d\n", 
		gc.total, gc.using, gc.total - gc.using);
	uintptr_t *ptr = (uintptr_t *)workspace;
	while(*ptr == 0)
		ptr++;
	_gc_mark((uintptr_t)ptr);
	_gc_sweep();
	printf("GC after=> total:%d, using:%d, remain:%d\n", 
		gc.total, gc.using, gc.total - gc.using);
}

void
init_gc(int size)
{
	size = _alignment(size);
	gc.total = size;
	gc.using = 0;
	gc.beg = (uintptr_t)xalloc(size);
	gc.end = gc.beg + size;
	gc.objs = xalloc(sizeof(OList));
}

static Object**
_new_Nil(void)
{
	Object *nil = _new(sizeof(void*) + 4, SYMBOL);
	_set_ptr(nil, &nil->sym);
	memcpy(nil->sym, "nil", 4);
	return _entry(nil);
}

void
init_predefined(void)
{
	symbols = Nil = _new_Nil();
	symbols = new_cons(Nil, symbols);

	True = new_symbol("true"); 
	False = new_symbol("false");
	Plus = new_symbol("+");
	Minus = new_symbol("-");
	Lambda = new_symbol("lambda");
	Car = new_symbol("car");
	Cdr = new_symbol("cdr");
	Quote = new_symbol("'");

	root_env = new_env(Nil, Nil);
	add_variable(True, True, root_env);
	add_variable(False, False, root_env);
	add_variable(Nil, Nil, root_env);

	init_primitive();
}
