#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <setjmp.h>

#define ALIGN (sizeof(int) * 2)
#define OBJ_SIZE ALIGN
#define IS_MARKED(x) ((x)->type & Obj_Marked)
#define SET_MARK(x) ((x)->type |=  Obj_Marked)
#define UNSET_MARK(x) ((x)->type &= (~Obj_Marked))
#define SET_SP(addr) {\
	uintptr_t _lololololol_ = 0;\
	addr = (uintptr_t)&_lololololol_;\
	}

typedef struct
{
	uintptr_t beg;
	uintptr_t end;
	int total;
	int using;
	uintptr_t top;
	uintptr_t bot;
	struct {
		Object **objs;
		int objs_cap;
		int objs_len;
	};
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

static void 
_entry(void *obj)
{
	if(gc.objs_len + 1 > gc.objs_cap){
		gc.objs = realloc(gc.objs, sizeof(Object*) * gc.objs_cap * 2);
		for(int i = gc.objs_cap; i < gc.objs_cap * 2; ++i)
			gc.objs[i] = 0;
		gc.objs_cap *= 2;
	}
	for(int i = 0; i< gc.objs_cap; ++i){
		if(gc.objs[i] == 0){
			gc.objs[i] = obj;
			gc.objs_len++;
			return;
		}
	}
	panic("unreachable");
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
	if(gc.using + size > gc.total/2) gc_run();
	assert(gc.using + size < gc.total);

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
	_entry(obj);
	printf("%p allocated %d byte\n", obj, obj->size);
	return obj;
}

Object*
new_int(long val)
{
	Object *obj = _new(sizeof(long), INT);
	obj->value = val;
	return obj;
}

Object*
new_cons(Object *car, Object *cdr)
{
	Object *obj = _new(sizeof(void*)*2, CELL);
	obj->car = car;
	obj->cdr = cdr;
	return obj;
}

Object*
new_env(Object *vars, Object *up)
{
	Object *obj = _new(sizeof(void*)*2, ENV);
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
	Object *obj = _new(sizeof(void*), PRIM);
	obj->fn = fn;
	return obj;
}

Object*
new_function(Object *env, enum Obj_Type type, Object *params, Object *body)
{
	Object *fn = _new(sizeof(void*)*3, type);
	fn->params = params;
	fn->body = body;
	fn->env = env;
	return fn;
}

static void
_set_ptr(Object *obj, void *dst)
{
	uintptr_t ptr = (uintptr_t)obj;
	ptr += OBJ_SIZE;
	ptr += sizeof(ptr);
	memcpy(dst, &ptr, sizeof(ptr));
}

Object*
new_symbol(char *sym)
{
	for(Object *c = symbols; c != Nil; c = c->cdr){
		if(strcmp(sym, c->car->sym) == 0)
			return c->car;
	}
	int objs_len = strlen(sym);
	Object *obj = _new(sizeof(void*) + objs_len + 1, SYMBOL);
	_set_ptr(obj, &obj->sym);
	memcpy(obj->sym, sym, objs_len + 1);
	symbols = new_cons(obj, symbols);
	return obj;
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
	for(int i=0; i<gc.objs_cap; ++i){
		Object *obj = gc.objs[i];
		if(obj == 0)
			continue;
		if(IS_MARKED(obj)==0){
			_init_object(obj);
			gc.objs[i] = 0;
			gc.objs_len--;
		}else{
			UNSET_MARK(obj);
		}
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

static Object*
_find(void *addr)
{
	for(int i = 0; i < gc.objs_cap; ++i){
		if(addr == gc.objs[i])
			return addr;
	}
	return 0;
}

static void
_gc_mark(void)
{
	SET_SP(gc.bot);
	_mark(symbols);
	_mark(root_env);
	for(uintptr_t ptr = gc.bot;
		ptr < gc.top;
		ptr += sizeof(uintptr_t))
	{
		Object *cur = _find(*(void**)ptr);
		if(cur)
			_mark(cur);
	}
}

void
gc_run(void)
{
	jmp_buf tmp;
	setjmp(tmp); /* push current register */
	printf("GC before=> total:%d, using:%d, remain:%d\n",
		gc.total, gc.using, gc.total - gc.using);
	_gc_mark(); 
	_gc_sweep();
	printf("GC after=> total:%d, using:%d, remain:%d\n", 
		gc.total, gc.using, gc.total - gc.using);
}

void
init_gc(int size)
{
	SET_SP(gc.top);
	SET_SP(gc.bot);
	gc.objs_cap = 1;
	gc.objs_len = 0;
	gc.objs = xalloc(sizeof(Object*) * gc.objs_cap);
	size = _alignment(size);
	gc.total = size;
	gc.using = 0;
	gc.beg = (uintptr_t)xalloc(size);
	gc.end = gc.beg + size;
}

static Object*
_new_Nil(void)
{
	Object *nil = _new(sizeof(void*) + 4, SYMBOL);
	_set_ptr(nil, &nil->sym);
	memcpy(nil->sym, "nil", 4);
	return nil;
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
	Cons = new_symbol("cons");
	Define = new_symbol("define");
	Setq = new_symbol("setq");

	root_env = new_env(Nil, Nil);
	add_variable(True, True, root_env);
	add_variable(False, False, root_env);
	add_variable(Nil, Nil, root_env);

	init_primitive();
}
