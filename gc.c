#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define DEFAULT_OBJ_SLOTS 64
#define OBJ_SIZE (sizeof(int)*2)
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
	void *beg;
	void *end;
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

static Object**
_entry(Object *obj)
{
	OList *last = 0;
	OList *p = gc.objs;
	while(p){
		for(int i = 0; i < sizeof(p->arr); ++i){
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

static Object*
_new(enum Obj_Type type, int size)
{
	if(size < sizeof(void*)) size=sizeof(void*);
	size += OBJ_SIZE;
	assert(size < gc.total);

	uint8_t *cur = gc.beg;
	while(1){
		if(cur+size >= (uint8_t*)gc.end)
			panic("Can't allocate %d byte", size);
		int i = 0;
		for(; i < size; ++i){
			if(cur[i])
				break;
		}
		if(i==size) break;
		cur += i;
		cur += *(int*)cur;
	}
	Object *obj = (Object*)cur;
	obj->type = type;
	obj->size = size;
	obj->mem = &cur[OBJ_SIZE + sizeof(void*)];
	gc.using += size;
	printf("allocated %d\n", obj->size);
	return obj;
}

Object*
new_int(int val)
{
	Object *obj = _new(INT, sizeof(void*));
	obj->value = val;
	return obj;
}

Object*
new_cons(Object *car, Object *cdr)
{
	Object *obj = _new(CELL, sizeof(void*)*2);
	obj->car = car;
	obj->cdr = cdr;
	return obj;
}

Object*
new_env(Object *vars, Object *up)
{
	Object *obj = _new(ENV, sizeof(void*)*2);
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
	Object *obj = _new(PRIM, sizeof(void*));
	obj->fn = fn;
	return obj;
}

Object*
new_function(Object *env, enum Obj_Type type, Object *params, Object *body)
{
	Object *fn = _new(type, sizeof(void*)*3);
	fn->params = params;
	fn->body = body;
	fn->env = env;
	return fn;
}

Object*
new_string(char *ptr)
{
	int len = strlen(ptr);
	len  = len <= 32 ? 32 : len;
	Object *s = _new(STRING, sizeof(void*)*3 + len + 1); 
	s->ptr = s->beg;
	s->end = s->ptr + len;
	if(ptr) str_puts(s, ptr);
	return s;
}

Object*
new_symbol(char *sym)
{
	for(Object *c = symbols; c != Nil; c = c->cdr){
		if(strcmp(sym, c->car->sym) == 0)
			return c->car;
	}
	int len = strlen(sym);
	Object *obj = _new(SYMBOL, sizeof(void*) + len + 1);
	memcpy(obj->sym, sym, len + 1);
	symbols = new_cons(obj, symbols);
	return obj;
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

void
init_gc(int size)
{
	gc.total = size;
	gc.using = 0;
	uint8_t *ptr = gc.beg = xalloc(size);
	gc.end = &ptr[size];
	gc.objs = xalloc(sizeof(OList));
}
