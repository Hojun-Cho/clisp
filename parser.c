#include "dat.h"
#include "fn.h" 
#include <ctype.h>
#include <string.h>

#define SYMBOL_LEN 64

const char symbolchars[] = "!*/%-=+<>'";

static Object* lparlist(FILE *);
static Object* list(FILE *);

static char
get(FILE *f)
{
	char c = fgetc(f);
	if(c == EOF)
		error("EOF");
	return c;
}

static void
expect(FILE *f, char x)
{
	char y = get(f);
	if(x != y)
		error("expected '%c', actual '%c'", x, y);
}

static char
lookup(FILE *f)
{
	char c = get(f);
	ungetc(c, f);
	return c;
}

/* skip space */
static char
slookup(FILE *f)
{
	char c = -1;
	while(1){
		c = get(f);
		if(isspace(c) == 0)
			break;
	}
	ungetc(c, f);
	return c;
}

static Object*
symbol(FILE *f, char c)
{
	char buf[SYMBOL_LEN+1] = {0,};
	int len = 0;
	buf[len++] = c;
	while(isalnum(lookup(f)) || strchr(symbolchars, lookup(f))){
		if(len >= sizeof(buf)-1)
			error("Symbol too long");
		buf[len++] = get(f);
	}
	buf[len] = 0;
	return newsymbol(gc, buf, len);
}

static long
number(FILE *f)
{
	long val = get(f) - '0';
	while(isdigit(lookup(f)))
		val = val * 10 + (get(f) - '0');
	return val;
}

static Object*
quote(FILE *f)
{
	Object *car = &Quote;
	Object *ccdr = list(f);
	Object *cdr = newcons(gc, ccdr, &Nil);
	return newcons(gc, car, cdr);
}

static Object*
string(FILE *f)
{
	Object *str = newstr(gc, 16);
	while(lookup(f) != '\"'){
		str = strputc(str, get(f));
	}
	expect(f, '\"');
	return str;
}

static Object*
atom(FILE *f, char c)
{
	if(isdigit(c))
		return newint(gc, number(f));
	get(f);
	if(c == '-'){
		if(isdigit(lookup(f)))
			return newint(gc, -number(f));
		else
			return symbol(f, '-');
	}
	if(c == '"'){
		return string(f);
	}
	if(isalpha(c) || strchr(symbolchars, c)){
		return symbol(f, c);
	}
	error("bad char in list '%c'", c);
	return 0;
}

static Object*
lparlist(FILE *f)
{
	Object *car = 0;
	Object *cdr = 0;
	char c = slookup(f);
	switch(c){
	case '\'':
		get(f);
		car = quote(f);
		cdr = lparlist(f);
		return newcons(gc, car, cdr);
	case '.':
		get(f);	
		return list(f);
	case '(':
		car = list(f);
		cdr = lparlist(f);
		return newcons(gc, car, cdr);
	case ')':
		return &Nil;
	}
	car = atom(f, c);
	cdr = lparlist(f);
	return newcons(gc, car ,cdr);
}

static Object*
list(FILE *f)
{
redo:
	char c = slookup(f);
	switch(c){
	case ';':
		skipline(f);
		goto redo;
	case '\'':
		get(f);
		return quote(f);
	case '(':{
		get(f);
		Object *obj = lparlist(f);
		slookup(f);
		expect(f, ')');
		return obj;
		}
	}	
	return atom(f, c);
}

void
skipline(FILE *f)
{
	for(;;){
		switch(get(f)){
		case '\n':
			return;
		case '\r':
			if(lookup(f) == '\n')
				get(f);
			return;
		}
	}
}

Object*
nextexpr(FILE *f)
{
	return list(f);
}
