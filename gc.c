#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>

enum
{
	USING = 1 << 1,

	OFFSET = sizeof(int),
};

int running = 0;

void
gccompact(int cap, GC *gc)
{
	GC *ngc = newgc((void*)gc->top, gc->cap * 2);
}

void
gcfree(GC *gc, void *src)
{
	int *p = (int*)src - 1;
	int sz = *p;
	memset(p, 0, sz);
	gc->using -= sz;
}

void
freeobj(GC *gc, Object *p)
{
	gc->using -= sizeof(Object);
	p->next  = 0;
	switch(p->type){
	default:
		break;
	case OSTRING:
	case OIDENT:
		gcfree(gc, p->beg);
		break;
	}
	memset(p, 0, sizeof(Object));
	if(gc->freed == 0)
		gc->freed = p;
	else{
		p->next = gc->freed;
		gc->freed = p;
	}
}

void*
gcalloc(GC *gc, int sz)
{
	sz += OFFSET;
	if(sz % OFFSET) sz = sz + OFFSET - (sz % OFFSET);
	for(u64 i = gc->sb; i < gc->se;){
		u64 j = i;
		for(;j - i < sz; j += OFFSET){
			if(*(int*)(j) != 0)
				break;
		}
		if(j - i == sz){
			gc->using += sz;
			*(int*)i = sz;
			i += OFFSET;
			return (void*)i;
		}
		i = j + *(int*)(j);
	}
	panic("gccalloc : Not impl yet raise");
}

void*
gcralloc(GC *gc, void *src, int sz)
{
	void *dst = gcalloc(gc, sz);
	int osz = ((int*)src)[-1];
	memcpy(dst, src, osz);
	gcfree(gc, src);
	return dst;
}

void
mark(GC *gc, Object *obj)
{
	if(obj == 0 || obj->flag&USING)
		return;
	obj->flag = USING;
	switch(obj->type){
	case OCELL:
		mark(gc, obj->car);
		mark(gc, obj->cdr);
		break;
	case OENV:
		mark(gc, obj->name);
		mark(gc, obj->vars);
		mark(gc, obj->up);
		break;
	case OFUNC:
		mark(gc, obj->params);
		mark(gc, obj->body);
		mark(gc, obj->env);
		break;
	}
}

void
gcsweep(GC *gc)
{
	Object *last = 0;
	for(Object *p = gc->objs; p;){
		if(p->flag&USING){
			p->flag = 0;
			last = p;
			p = p->next;
		}else{
			Object *tmp = p;
			if(last == 0){
				gc->objs = p->next;
			}else{
				last->next = p->next;
			}
			p = p->next;
			freeobj(gc, tmp);
		}
	}
}

int
isobj(GC *gc, u64 p)
{
	if(gc->ob <= p && p < gc->oe){
		p -= gc->ob;
		return (p % sizeof(Object)) == 0;
	}
	return 0;
}

void
gcmark(GC *gc)
{
	void *_ = 0;
	u64 bot = (u64)&_;
	for(; bot < gc->top; bot += sizeof(bot)){
		u64 val = (u64)*(void**)bot;
		if(isobj(gc, val))
			mark(gc, (Object*)val);
	}
}

void
gcrun(GC *gc)
{
	running = 1;
	printf("before=> cap:%d using:%d remain:%d\n", gc->cap, gc->using, gc->cap-gc->using);
	jmp_buf reg;
	setjmp(reg);
	gcmark(gc);
	gcsweep(gc);
	printf("after=> cap:%d using:%d remain:%d\n", gc->cap, gc->using, gc->cap-gc->using);
	running = 0;
}

Object*
newobj(GC *gc, enum OType type)
{	
	if(gc->op + sizeof(Object) >= gc->oe){
		panic("Not impl yet newobj raise");	
	}
	gcrun(gc);
	gc->using += sizeof(Object);
	Object *r = 0;
	if(gc->freed){
		r = gc->freed;
		gc->freed = gc->freed->next;
	}else{
		r = (Object*)gc->op;
		gc->op += sizeof(Object);
	}
	r->type = type;
	if(gc->objs == 0)
		gc->objs = r;
	else{
		r->next = gc->objs;
		gc->objs = r;
	}
	return r;
}

GC*
newgc(void *top, int cap)
{
	GC *gc = calloc(1, sizeof(GC));
	if(gc == 0)
		panic("can't alloc %d byte\n", sizeof(GC));
	gc->top = (u64)top;
	if((gc->memory = malloc(cap)) == 0)
		panic("can't alloc %d byte\n", cap);
	gc->cap = cap;
	gc->using = 0;

	gc->op = gc->ob = (u64)gc->memory;
	gc->oe = gc->op + (float)cap * 0.64;

	gc->sb = (u64)gc->memory + (float)cap * 0.64;
	gc->se = (u64)gc->memory + cap;
	return gc;
}
