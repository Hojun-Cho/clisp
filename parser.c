#include "dat.h"
#include "fn.h" 
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define SYMBOL_LEN 64

const char symbolchars[] = "!*/%-=+<>'";

static Object* lparlist(void);
static Object* list(void);

static char
get(void)
{
	char c = getchar();
	if(c == EOF)
		panic("EOF");
	return c;
}

static void
expect(char x)
{
	char y = get();
	if(x != y)
		error("expected '%c', actual '%c'", x, y);
}

static char
lookup(void)
{
	char c = get();
	ungetc(c, stdin);
	return c;
}

/* skip space */
static char
slookup(void)
{
	char c = -1;
	while(1){
		c = get();
		if(isspace(c) == 0)
			break;
	}
	ungetc(c, stdin);
	return c;
}

static Object*
symbol(char c)
{
	char buf[SYMBOL_LEN+1] = {0,};
	int len = 0;
	buf[len++] = c;
	while(isalnum(lookup()) || strchr(symbolchars, lookup())){
		if(len >= sizeof(buf)-1)
			error("Symbol too long");
		buf[len++] = get();
	}
	buf[len] = 0;
	return newsymbol(gc, buf, len);
}

static long
number(void)
{
	long val = get() - '0';
	while(isdigit(lookup()))
		val = val * 10 + (get() - '0');
	return val;
}

static Object*
quote(void)
{
	Object *car = &Quote;
	Object *ccdr = list();
	Object *cdr = newcons(gc, ccdr, &Nil);
	return newcons(gc, car, cdr);
}

static Object*
string(void)
{
	Object *str = newstr(gc, 16);
	while(lookup() != '\"'){
		strputc(str, get());
	}
	expect('\"');
	return str;
}

static Object*
atom(char c)
{
	if(isdigit(c))
		return newint(gc, number());
	get();
	if(c == '-'){
		if(isdigit(lookup()))
			return newint(gc, -number());
		else
			return symbol('-');
	}
	if(c == '"'){
		return string();
	}
	if(isalpha(c) || strchr(symbolchars, c)){
		return symbol(c);
	}
	error("bad char in list '%c'", c);
	return 0;
}

static Object*
lparlist(void)
{
	Object *car = 0;
	Object *cdr = 0;
	char c = slookup();
	switch(c){
	case '\'':
		get();
		car = quote();
		cdr = lparlist();
		return newcons(gc, car, cdr);
	case '.':
		get();	
		return list();
	case '(':
		car = list();
		cdr = lparlist();
		return newcons(gc, car, cdr);
	case ')':
		return &Nil;
	}
	car = atom(c);
	cdr = lparlist();
	return newcons(gc, car ,cdr);
}

static Object*
list(void)
{
	char c = slookup();
	switch(c){
	case '\'':
		get();
		return quote();
	case '(':{
		get();
		Object *obj = lparlist();
		slookup();
		expect(')');
		return obj;
		}
	}	
	return atom(c);
}

void
skipline(void)
{
	for(;;){
		switch(get()){
		case '\n':
			return;
		case '\r':
			if(lookup() == '\n')
				get();
			return;
		}
	}
}

Object*
nextexpr(void)
{
	return list();
}
