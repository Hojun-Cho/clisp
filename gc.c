#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>

enum
{
	USING = 1 << 1,
	FORWARD = 1 << 2,

	OFFSET = sizeof(int),
};

int
isobj(GC *gc, u64 p)
{
	if(gc->ob <= p && p < gc->oe){
		p -= gc->ob;
		return (p % sizeof(Object)) == 0;
	}
	return 0;
}

Object*
cloneobj(GC *dst, GC *src, Object *obj)
{
	if(obj == 0)
		return 0;
	if(obj->type==OBLTIN||obj->type==OSYMBOL)
		return obj;
	if(obj->flag&FORWARD)
		return obj->forward;
	Object *p = newobj(dst, obj->type);
	obj->flag |= FORWARD;
	obj->forward = p;
	switch(obj->type){
	case OINT:
		p->num = obj->num;
		break;
	case OSTRING:
	case OIDENT:
		int len = obj->ptr - obj->beg;
		p->beg = gcalloc(dst, len + 1);
		p->end = p->ptr = p->beg + len;
		memcpy(p->beg, obj->beg, len + 1);
		break;
	case OCELL:
		p->car = cloneobj(dst, src, obj->car);
		p->cdr = cloneobj(dst, src, obj->cdr);
		break;
	case OENV:
		p->name = cloneobj(dst, src, obj->name);
		p->vars = cloneobj(dst, src, obj->vars);
		p->up = cloneobj(dst, src, obj->up);
		break;
	case OFUNC:
		p->params = cloneobj(dst, src, obj->params);
		p->body = cloneobj(dst, src, obj->body);
		p->env = cloneobj(dst, src, obj->env);
		break;
	}
	return p;
}

Object*
forwardobj(Object *p)
{
	if(p == 0)
		return 0;
	if(p->flag&FORWARD)
		return p->forward;
	switch(p->type){
	case OBLTIN:
	case OSYMBOL:
		break;
	case OCELL:
		p->car = forwardobj(p->car);
		p->cdr = forwardobj(p->cdr);
		break;
	case OENV:
		p->name = forwardobj(p->name);
		p->vars = forwardobj(p->vars);
		p->up = forwardobj(p->up);
		break;
	case OFUNC:
		p->params = forwardobj(p->params);
		p->body = forwardobj(p->body);
		p->env = forwardobj(p->env);
		break;
	}
	return p;
}

void
compact(u64 bot, GC *dst, GC *src)
{
	for(Object *p=src->objs; p; p=p->next){
		cloneobj(dst, src, p);
	}
	for(; bot < src->top; bot += sizeof(bot)){
		u64 val = (u64)*(void**)bot;
		if(isobj(src, val)){
			Object *obj = (Object*)val;
			if(obj->flag&FORWARD)
				*(void**)bot = forwardobj(obj);
		}
	}
}

void
gccompact(int cap, GC *src)
{
	void *_ = 0;
	u64 bot = (u64)&_;
	GC *dst = newgc((void*)src->top, cap);
	dst->running = 1;
	compact(bot, dst, src);
	dst->running = 0;
	free(src->memory);
	*src = *dst;
	free(dst);
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
			p->flag &= ~(USING);
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
gcrun(GC *src)
{
	if(src->running)
		return;
	src->running = 1;
	printf("before=> cap:%d using:%d remain:%d\n", gc->cap, gc->using, gc->cap-gc->using);
	jmp_buf reg;
	setjmp(reg);
	gcmark(src);
	gcsweep(src);
	gccompact(src->cap + 500, src);
	printf("after=> cap:%d using:%d remain:%d\n", gc->cap, gc->using, gc->cap-gc->using);
	src->running = 0;
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
	if((gc->memory = calloc(1, cap)) == 0)
		panic("can't alloc %d byte\n", cap);
	gc->cap = cap;
	gc->using = 0;

	gc->op = gc->ob = (u64)gc->memory;
	gc->oe = gc->op + (float)cap * 0.64;

	gc->sb = (u64)gc->memory + (float)cap * 0.64;
	gc->se = (u64)gc->memory + cap;
	return gc;
}
