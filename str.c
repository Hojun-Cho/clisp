#include "dat.h"
#include "fn.h"
#include <string.h>

static void
raise(Object *s, int ns)
{
	int pos = s->ptr - s->beg;
	char *ptr = xralloc(s->beg, ns + 1);
	s->beg = ptr;
	s->ptr = s->beg + pos;
	s->end = s->beg + ns;
}

void
strputc(Object *s, int c)
{
	if(s->ptr >= s->end)
		raise(s, (s->end - s->beg) * 2);
	*s->ptr++ = c;
	*s->ptr = 0;
}

void
strputs(Object *s, char *ptr)
{
	int l = strlen(ptr);
	if(s->ptr + l >= s->end)
		raise(s, s->end - s->beg + l);
	memcpy(s->ptr, ptr, l);
	s->ptr += l;
	s->ptr[0] = 0;
}

int
strequal(Object *a, Object *b)
{
	int la = a->ptr - a->beg;
	int lb = b->ptr - b->beg;
	return la == lb && memcmp(a->beg, b->beg, la) == 0;
}
