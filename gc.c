#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdint.h>
#include <string.h>

enum
{
	USING = 1 << 1,
};

typedef struct
{
	int total;
	int using;
	uintptr_t top;
	uintptr_t beg;
	uintptr_t end;
	Object objs;
	Object freed;
}GC;

GC gc = {0};

static void
pushobj(Object *list, Object *obj)
{
	Object *l = list;
	Object *c = l->next;
	while(c){
		l = c;
		c=c->next;
	}
	l->next = obj;
}

static void
mark(Object *obj)
{
	if(obj == 0 || obj->flag & USING)
		return;
	obj->flag = USING;
	switch(obj->type){
	case OCELL:
		mark(obj->car);
		mark(obj->cdr);
		break;
	case OENV:
		mark(obj->name);
		mark(obj->vars);
		mark(obj->up);
		break;
	case OFUNC:
		mark(obj->params);
		mark(obj->body);
		mark(obj->env);
		break;
	}
}

static int
isobj(uintptr_t val)
{
	if(val < gc.beg || val >= gc.end)
		return 0;
	val -= gc.beg;
	uintptr_t mod = val % sizeof(Object);
	return mod == 0;
}

static void
freeobj(Object *obj)
{
	switch(obj->type){
	case OSTRING:
	case OIDENT:
		printf("freed => '%s'\n", obj->beg);
		free(obj->beg);
		break;
	}
	memset(obj, 0, sizeof(*obj));
	pushobj(&gc.freed, obj);
	gc.using -= sizeof(Object);
}

static void
gcsweep(void)
{
	Object *l = &gc.objs;
	Object *c = l->next;
	while(c){
		if(c->flag&USING){
			c->flag = 0;
			l = c;
			c = c->next;
			continue;
		}
		Object *t = c;
		l->next = c->next;
		c = c->next;
		freeobj(t);
	}
}

static void
gcmark(void)
{
	void *_ = 0;
	uintptr_t bot = (uintptr_t)&_;
	for(; bot < gc.top; bot += sizeof(bot)){
		uintptr_t val = (uintptr_t)*(void**)bot;
		if(isobj(val))
			mark((Object*)val);
	}
}

void
gcrun(void)
{
	jmp_buf reg;
	setjmp(reg);
	gcmark();
	gcsweep();
	gcstatus();
}

void
gcstatus(void)
{
	printf("curren=> total:%d using:%d remain:%d\n", gc.total, gc.using, gc.total-gc.using);
}

void*
xalloc(int sz)
{
	int *res = calloc(1, sz);
	if(res == 0)
		panic("Can't allocated %d byte", sz);
	return res;
}

void*
xralloc(void *src, int sz)
{
	int *p = realloc(src, sz);
	if(p == 0)
		panic("Can't allocated %d byte", sz);
	return p;
}

Object* 
newobj(enum OType type)
{
	gcrun();
	Object *obj = 0;
	if(gc.freed.next){
		obj = gc.freed.next;
		gc.freed.next = obj->next;
		obj->next = 0;
	}else
		panic("not impl yet");
	obj->type = type;
	pushobj(&gc.objs, obj);
	gc.using += sizeof(Object);
	return obj;
}

void
gcinit(void *top, int cap)
{
	gc.total = cap;
	gc.using = 0;
	gc.top = (uintptr_t)top;
	gc.beg = (uintptr_t)xalloc(cap);
	gc.end = gc.beg + cap;
	Object *p = &gc.freed;
	for(uintptr_t i = gc.beg; i < gc.end; i+=sizeof(Object)){
		p = p->next = (Object*)i;
	}
}
