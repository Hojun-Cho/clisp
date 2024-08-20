#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <string.h>

Object*
new_string(char *ptr, int fixed)
{
	Object *s = new_object(Obj_String); 
	SET_ATOM(s);
	int len = strlen(ptr);
	len = len ? len : STR_DEFAULT_LEN;
	s->ptr = s->beg = xalloc(len);
	s->end = s->ptr + len;
	if(ptr) str_puts(s, ptr);
	s->fixed = fixed;
	return s;
}

static void
str_raise(Object *s, int ns)
{
	if(s->fixed) panic("Can't raise fixed string '%s'", s->beg);
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
	if(s->fixed) panic("Can't putc fixed string '%s'", s->beg);
	if(s->ptr + 1 >= s->end) str_raise(s, (s->end - s->beg) * 2);
	*s->ptr++ = c;
	*s->ptr = 0;
}

void
str_puts(Object *s, char *ptr)
{
	if(s->fixed) panic("Can't puts fixed string '%s'", s->beg);
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
