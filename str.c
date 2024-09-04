#include "dat.h"
#include "fn.h"
#include <string.h>

void
strinit(Object *s, Object *p)
{
	for(char *c = p->beg ; c < p->ptr;)
		*s->ptr++ = *c++;
	*s->ptr = 0;
}

Object*
strraise(Object *s, int ns)
{
	Object *dst = newstr(gc, ns + 1);
	strinit(dst, s);
	return dst;
}

Object*
strputc(Object *s, int c)
{
	if(s->ptr + 1 >= s->end)
		s = strraise(s, (s->end - s->beg) * 2);
	*s->ptr++ = c;
	*s->ptr = 0;
	return s;
}

Object*
strputs(Object *s, Object *ptr)
{
	int l = ptr->ptr - ptr->beg;
	if(s->ptr + l>= s->end)
		s = strraise(s, s->end - s->beg + l);
	memcpy(s->ptr, ptr->beg, l);
	s->ptr += l;
	*s->ptr = 0;
	return s;
}

int
strequal(Object *a, Object *b)
{
	int la = a->ptr - a->beg;
	int lb = b->ptr - b->beg;
	return la == lb && memcmp(a->beg, b->beg, la) == 0;
}