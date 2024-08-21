#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <string.h>

static void
str_raise(Object *s, int ns)
{
	panic("Not impl yet");
	int pos = s->ptr - s->beg;
	char *ptr = realloc(s->beg, ns);
	if(ptr == 0) panic("realloc fail %dbyte", ns);
	s->beg = ptr;
	s->ptr = s->beg + pos;
	s->end = s->beg + ns;
}

void
str_putc(Object *s, int c)
{
	if(s->ptr + 1 >= s->end) str_raise(s, (s->end - s->beg) * 2);
	*s->ptr++ = c;
	*s->ptr = 0;
}

void
str_puts(Object *s, char *ptr)
{
	int l = strlen(ptr);
	if(s->ptr + l >= s->end)
		str_raise(s, s->end - s->beg + l);
	memcpy(s->ptr, ptr, l);
	s->ptr += l;
	s->ptr[0] = 0;
}

int
str_len(Object *a)
{
	if(a == 0)
		return 0;
	return a->ptr - a->beg;
}

int
str_cmp(Object *a, Object *b)
{
	if(a == 0 || b == 0) return -1;
	if(str_len(a) != str_len(b)) return -1;
	return memcmp(a->beg, b->beg, str_len(a));
}
