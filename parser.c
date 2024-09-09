#include "dat.h"
#include "fn.h" 
#include <ctype.h>
#include <string.h>

#define SYMBOL_LEN 64

const char symbolchars[] = "!*/%-=+<>'";

static Object* lparlist(FILE *, int *);
static Object* list(FILE *, int *);

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
quote(FILE *f, Object *car, int *bq)
{
	Object *ccdr = list(f, bq);
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
lparlist(FILE *f, int *bq)
{
	Object *car = 0;
	Object *cdr = 0;
	Object *res = 0;
	char c = slookup(f);
	switch(c){
	case '`':
		*bq += 1;
		get(f);
		car = quote(f, &Bquote, bq);
		cdr = lparlist(f, bq);
		res = newcons(gc, car, cdr);
		*bq -= 1;
		return res;
	case '\'':
		get(f);
		car = quote(f, &Quote, bq);
		cdr = lparlist(f, bq);
		return newcons(gc, car, cdr);
	case ',':
		if(*bq <= 0)
			error("comma is illegal outside of backquote");
		get(f);
		if(lookup(f) == '@'){
			get(f);
			res = newcons(gc, &Comma, &Nil);
			res->cdr = newcons(gc, &Splice, list(f, bq));
			return newcons(gc, res, lparlist(f, bq));
		}
		car = newcons(gc, &Comma, list(f, bq));
		cdr = lparlist(f, bq);
		return newcons(gc, car, cdr);
	case '.':
		get(f);	
		return list(f, bq);
	case '(':
		car = list(f, bq);
		cdr = lparlist(f, bq);
		return newcons(gc, car, cdr);
	case ')':
		return &Nil;
	default:
		car = atom(f, c);
		cdr = lparlist(f, bq);
		return newcons(gc, car ,cdr);
	}
}

static Object*
list(FILE *f, int *bq)
{
redo:
	Object *res = 0;
	char c = slookup(f);
	switch(c){
	case ';':
		get(f);
		skipline(f);
		goto redo;
	case '`':
		*bq += 1;
		get(f);
		res = quote(f, &Bquote, bq);
		*bq -= 1;
		return res;
	case ',':
		if(*bq <= 0)
			error("comma is illegal outside of backquote");
		get(f);
		if(lookup(f) == '@'){
			get(f);
			res = newcons(gc, &Comma, &Nil);
			res->cdr = newcons(gc, &Splice, list(f, bq));
		}else
			res = newcons(gc, &Comma, list(f, bq));
		return res;
	case '\'':
		get(f);	
		return quote(f, &Quote, bq);
	case '(':{
		get(f);
		res = lparlist(f, bq);
		slookup(f);
		expect(f, ')');
		return res;
		}
	default:
		return atom(f, c);
	}	
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
	int bq = 0;
	Object *expr = list(f, &bq);
	if(bq != 0){
		error("Bad backquote in expr");	
	}
	return expr;
}
