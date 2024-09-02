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
	default: panic("unreachable");
	case OINT:
		p->num = obj->num;
		break;
	case OSTRING:
	case OIDENT:{
            int len = obj->ptr - obj->beg;
            p->beg = gcalloc(dst, len + 1);
            p->end = p->ptr = p->beg + len;
            memcpy(p->beg, obj->beg, len + 1);
            break;
        }
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
moveobj(Object *p)
{
	if(p == 0 || p->type == 0)
		return 0;
	switch(p->type){
	default:
		return p->forward;
	case OBLTIN:
	case OSYMBOL:
		return p;
	}
}

void
forwardstack(u64 bot, GC *dst, GC *src)
{
	u64 pos, diff, *stk;
	Object *moved, *org;
	for(; bot < src->top; bot += sizeof(bot)){
		stk = (u64*)(void**)bot;
		if(src->ob <= *stk && *stk < src->oe){
			diff = (*stk - src->ob) % sizeof(Object);
			org = (Object*)(*stk - diff);
			if((moved = moveobj(org)) == 0)
				continue;
			diff = (u64)org - *stk;
			pos = (u64)moved + diff;
			memcpy(stk, &pos, sizeof(pos));
		}
		else if(src->sb <= *stk && *stk < src->se)
		for(org = src->objs; org; org = org->next){
			if(org->type == OSTRING || org->type == OIDENT){
				u64 beg = (u64)org->beg - OFFSET;
				u64 end = beg + *(int*)beg;
				if(beg <= *stk &&  *stk < end){
					moved = moveobj(org);
					diff = (*stk - beg);
					pos = (u64)moved->beg - OFFSET + diff;
					memcpy(stk, &pos, sizeof(pos));
					break;
				}
			}
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
	for(Object *p=src->objs; p; p=p->next)
		cloneobj(dst, src, p);
	forwardstack(bot, dst, src);
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
	return 0;
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
	if(obj == 0 || obj->flag&USING ||
		obj->type == 0|| obj->type==OBLTIN ||obj->type==OSYMBOL)
		return;
	obj->flag = USING;
	switch(obj->type){
	default:break;
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
		if(p->type == 0|| p->type==OBLTIN ||p->type==OSYMBOL)
			return;
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
	u64 _ = 0, stk, diff;
	u64 bot = (u64)&_;
	Object *obj;
	for(; bot < gc->top; bot += sizeof(bot)){
		stk = (u64)*(void**)bot;
		if(gc->ob <= stk && stk <= gc->oe){
			diff = (stk - gc->ob) % sizeof(Object);
			obj = (Object*)(stk - diff);
			mark(gc, obj);
		}
		else if(gc->sb <= stk && stk <= gc->se)
		for(Object *obj = gc->objs; obj; obj = obj->next)
		if(obj->type == OSTRING || obj->type == OIDENT){
			u64 beg = (u64)obj->beg - OFFSET;
			u64 end = beg + *(int*)beg;
			if(beg <= stk &&  stk < end){
				mark(gc, obj);
				break;
			}
		}
	}
}

void
gcrun(GC *src)
{
	if(src->running)
		return;
	printf("BEFORE=> cap:%10ld using:%10ld remain:%10ld\n", src->cap, src->using, src->cap - src->using);
	src->running = 1;
	jmp_buf reg;
	if(setjmp(reg)==1){
		printf("AFTER => cap:%10ld using:%10ld remain:%10ld\n", src->cap, src->using, src->cap - src->using);
		src->running = 0;
		return;
	}
	gcmark(src);
	gcsweep(src);
	gccompact(src->cap + 500, src);
	longjmp(reg, 1);
}

Object*
newobj(GC *gc, enum OType type)
{
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
	if(gc->objs)
		r->next = gc->objs;
	return gc->objs = r;
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
