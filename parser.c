#include "dat.h"
#include "fn.h" 
#include <stdio.h>
#include <ctype.h>
#include <string.h>

char symbol_chars[] = "~!@#$%^&*-_=+:/?<>";

static void
_assert(char x)
{
	char y = getchar();
	if(x != y)
		panic("expected '%c', actual '%c'", x, y);
}

static char
_lookup(void)
{
	char c = getchar();
	ungetc(c, stdin);
	return c;;
}

/* skip space */
static char
_slookup(void)
{
	char c = -1;
	while(1){
		c = getchar();
		if(isspace(c) == 0)
			break;
	}
	ungetc(c, stdin);
	return c;
}

void
skip_line(void)
{
	for(;;){
		switch(getchar()){
		case EOF:
		case '\n':
			return;
		case '\r':
			if(_lookup() == '\n')
				getchar();
			return;
		}
	}
}

static Object** _lpar_list(void);
static Object** _list(void);

static Object**
_symbol(char c)
{
	char buf[SYMBOL_MAX_LEN + 1] = {0,};
	int len = 0;
	buf[len++] = c;
	while(isalnum(_lookup()) || strchr(symbol_chars, _lookup())){
		if(len >= SYMBOL_MAX_LEN)
			error("Symbol too long");
		buf[len++] = getchar();
	}
	buf[len] = 0;
	return new_symbol(buf);
}

static long
_number()
{
	long val = getchar() - '0';
	while(isdigit(_lookup()))
		val = val * 10 + (getchar() - '0');
	return val;
}

static Object**
_quote(void)
{
	Object **car = Quote;
	Object **ccdr = _list();
	Object **cdr = new_cons(ccdr, Nil);
	return new_cons(car, cdr);
}

static Object**
_atom(char c)
{
	if(isdigit(c))
		return new_int(_number());
	if(c == '-'){
		getchar();
		if(isdigit(_lookup())) return new_int(-_number());
		else return _symbol('-');
	}
	if(isalpha(c) ||  strchr(symbol_chars, c)){
		getchar();
		return _symbol(c);
	}
	error("bad char in list '%c'", c);
}

static Object**
_lpar_list(void)
{
	Object **car = Nil, **cdr = Nil;
	char c = _slookup();
	switch(c){
	case '\'':
		getchar();
		car = _quote();
		cdr = _lpar_list();
		return new_cons(car, cdr);
	case '.':
		getchar();	
		return _list();
	case '(':
		car = _list();
		cdr = _lpar_list();
		return new_cons(car, cdr);
	case ')':
		return Nil;
	}
	car = _atom(c);
	cdr = _lpar_list();
	return new_cons(car ,cdr);
}

static Object**
_list(void)
{
	char c = _slookup();
	switch(c){
	case '\'':
		getchar();
		return _quote();
	case '(':{
		getchar();
		Object **res = _lpar_list();
		_slookup();
		_assert(')');
		return res;
		}
	}	
	return _atom(c);
}

Object**
next_expr(void)
{
	return _list();
}
