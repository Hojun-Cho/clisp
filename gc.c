#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

struct GC
{
	int running;
	int cap;
	int using;
	uintptr_t *top;
	uintptr_t *beg;
	uintptr_t *end;
};

enum
{
	USING = 1 << 1,
	FORWARD = 1 << 2,

	PTR = sizeof(void*),
};

static Object*
findobj(GC *gc, uintptr_t *stk)
{
	if(gc->beg <= stk && stk < gc->end)
		for(uintptr_t *p = gc->beg; p < stk;){
			if(*p){
				if(p <= stk && stk < p + *p) return (Object*)&p[1];
				else p += *p;
			}else{
				p++;
			}
		}
	return 0;
}

Object*
cloneobj(GC *dst, GC *src, Object *obj)
{
	if(obj->type==OBLTIN||obj->type==OSYMBOL) return obj;
	if(obj->flag&FORWARD) return obj->forward;

	Object *p;
	obj->flag |= FORWARD;
	switch(obj->type){
	default: panic("unreachable");
	case OINT:
		obj->forward = p = newint(dst, obj->num);
		break;
	case OSTRING:
		obj->forward = p = newstr(dst, obj->end - obj->beg);
		strinit(p, obj);
		break;
	case OIDENT:
		obj->forward = p = newsymbol(dst, obj->beg, obj->ptr - obj->beg);
		break;
	case OCELL:
		obj->forward = p = newcons(dst, &Nil, &Nil);
		p->car = cloneobj(dst, src, obj->car);
		p->cdr = cloneobj(dst, src, obj->cdr);
		break;
	case OENV:
		obj->forward = p = newenv(dst, &Nil, &Nil, &Nil);
		p->name = cloneobj(dst, src, obj->name);
		p->vars = cloneobj(dst, src, obj->vars);
		p->up = cloneobj(dst, src, obj->up);
		break;
	case OMACRO:
	case OFUNC:
		obj->forward = p = newfn(dst, &Nil, &Nil, &Nil, obj->type);
		p->params = cloneobj(dst, src, obj->params);
		p->body = cloneobj(dst, src, obj->body);
		p->env = cloneobj(dst, src, obj->env);
		break;
	}
	return p;
}

void
gcraise(GC *src)
{
	jmp_buf reg;
	uintptr_t _ = 0;
	if(src->running) return;
	if(setjmp(reg) == 1) return;

	GC *dst = newgc(src->top, src->cap * 2);
	src->running = dst->running = 1;
	for(uintptr_t *p = src->beg; p < src->end;)
		if(*p){
			cloneobj(dst, src, (Object*)&p[1]);
			p += *p;
		}else{
			p++;
		}
	for(uintptr_t *bot = &_; bot < src->top; bot++){
		Object *p = findobj(src, (uintptr_t*)*bot);
		if(p){
			uintptr_t diff = (uintptr_t)p - *bot;
			uintptr_t pos = (uintptr_t)p->forward + diff;
			memcpy(bot, &pos, PTR);
		}
	}

	free((void*)src->beg);
	dst->running = 0;
	*src = *dst;
	free(dst);
	longjmp(reg, 1);
}

void
mark(GC *gc, Object *obj)
{
	if(obj->flag&USING||obj->type==ONONE||obj->type==OSYMBOL||obj->type==OBLTIN)
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
	case OMACRO:
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
	for(uintptr_t *p = gc->beg; p < gc->end;)
		if(*p){
			Object *obj = (Object*)&p[1];
			if(obj->flag & USING){
				obj->flag = 0;
				p += p[0];
			}else{
				uintptr_t sz = *p;
				memset(p, 0, sz*PTR);
				gc->using -= sz*PTR;
				p += sz;
			}
		}else{
			p++;
		}
}

void
gcmark(GC *gc)
{
	uintptr_t _ = 0;
	for(uintptr_t *bot = &_; bot < gc->top; bot++){
		Object *obj = findobj(gc, (uintptr_t*)*bot);
		if(obj)
			mark(gc, obj);
	}
}

void
gcrun(GC *gc)
{
	if(gc->running)
		return;
	printgc("gc-beg", gc);
	gc->running = 1;
	jmp_buf reg;
	if(setjmp(reg)==1){
		gc->running = 0;
		printgc("gc-end", gc);
		return;
	}
	gcmark(gc);
	gcsweep(gc);
	longjmp(reg, 1);
}

static void*
alloc(GC *gc, int sz)
{
	sz /= PTR;
	for(uintptr_t *p = gc->beg; p < gc->end - sz;){
		int i = 0;
		for(; p[i] == 0 && i < sz; ++i)
			;
		if(i == sz){
			*p = sz;
	 		gc->using += sz*PTR;
			return p+1;
		}
		if(p[i]) p = &p[i] + p[i];
		else p = &p[i] + 1;
	}
	return 0;
}

void*
gcalloc(GC *gc, int sz)
{
	sz += PTR;
	if(sz%PTR) sz = sz + PTR - sz % PTR;
	if(gc->using + sz > gc->cap * 0.64) gcrun(gc);

	void *p = alloc(gc, sz);
	if(p)
		return p;
	gcraise(gc);
	p = alloc(gc, sz);
	if(p)
		return p;
	panic("newobj:can't alloc %d byte", sz);
	return 0;
}

void
printgc(char *prefix, GC *gc)
{
	printf("%*s=> cap:%10d using:%10d remain:%10d\n", 10, prefix, gc->cap, gc->using, gc->cap - gc->using);
}

GC*
newgc(void *top, int cap)
{
	GC *gc = calloc(1, sizeof(GC));
	if(gc == 0)
		panic("can't alloc %d byte\n", sizeof(GC));
	gc->cap = cap;
	gc->top = top;
	if((gc->beg = calloc(1, cap)) == 0)
		panic("can't alloc %d byte\n", cap);
	gc->end = gc->beg + cap/PTR;
	return gc;
}
