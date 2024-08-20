#include "dat.h"
#include "fn.h" 
#include <stdio.h>
#include <ctype.h>
#include <string.h>

static Object* _read_expr(void);

char symbol_chars[] = "~!@#$%^&*-_=+:/?<>";

static void
SExprint(Object *obj)
{
	if(obj == 0){
		printf("Nil");
		return;
	}
	switch(TYPE(obj)){
	case Obj_Cell:
		printf("(");
		SExprint(obj->car);
		printf(" . ");
		SExprint(obj->cdr);
		printf(")");
		return;
#define CASE(type, ...)                         \
    case type:                                  \
        printf(__VA_ARGS__);                    \
        return
    CASE(Obj_Int, "%d", obj->value);
	CASE(Obj_String, "\"%s\"", obj->beg);
    CASE(Obj_Symbol, "%s", obj->sym);
	CASE(Obj_Map,    "Map");
	CASE(Obj_Env,    "Env");
    CASE(Obj_True,   "true");
    CASE(Obj_False,  "false");
	CASE(Obj_Lambda, "<lambda>");
	CASE(Obj_Func,   "<func>");
	CASE(Obj_Nil,  "Nil");
#undef CASE
    default:
        error("SExpr=> print: Unknown type %d", TYPE(obj));
    }
}

void
print_expr(Object *obj)
{
	SExprint(obj);
	printf("\n");
}

static char
_getchar()
{
	char c = getchar();
	if(c == 0 || c == -1)
		panic("EOF");
	return c;
}

static char
_peek(void)
{
	char c = getchar();
	ungetc(c, stdin);
	return c;
}

Object*
reverse(Object *p)
{
	Object *ret = Nil;	
	for(;p != Nil;){
		Object *head = p;
		p = p->cdr;
		head->cdr = ret;
		ret = head;
	}
	return ret;
}

void
skip_line(void)
{
	for(;;){
		int c = _getchar();
		if(c == EOF || c == '\n')
			return;
		if(c == '\r'){
			if(_peek() == '\n')
				_getchar();
			return;
		}
	}
}

static Object*
_read_list(void)
{
	Object *obj = Nil, *head = Nil;
	for(;;){
		obj = _read_expr();
		if(obj == Cparen)
			return reverse(head);
		head = new_cons(obj, head);
	}
}

static Object*
_read_quote(void)
{
	Object *sym = new_symbol("quote");
	Object *tmp = _read_expr();
	tmp = new_cons(tmp, Nil);
	tmp = new_cons(sym, tmp);
	return tmp;
}

/* all object list */

static int
_read_number(int val)
{
	while(isdigit(_peek()))
		val = val * 10 + (_getchar() - '0');
	return val;
}

static Object*
_read_string(void)
{
	Object *obj = new_string("", 0);
	while(_peek() != '"')
		str_putc(obj, _getchar());
	if(_getchar() != '"')
		error("unclosed string !");
	return obj;	
}

static Object*
_read_symbol(char c)
{
	char buf[SYMBOL_MAX_LEN + 1] = {0,};
	int len = 0;
	buf[len++] = c;
	while(isalnum(_peek()) || strchr(symbol_chars, _peek())){
		if(len >= SYMBOL_MAX_LEN)
			error("Symbol too long");
		buf[len++] = _getchar();
	}
	buf[len] = 0;
	return new_symbol(buf);
}

static Object*
_read_expr(void)
{
	for(;;){
		int c = _getchar();
		if(isspace(c)) continue;
		if(c == EOF) return Nil;
		if(c == ';'){
			skip_line();
			continue;
		}
		if(c == '(') return _read_list();
		if(c == ')') return Cparen;
		if(c == '\'') return _read_quote();
		if(c == '"') return _read_string();
		if(isdigit(c)) return new_int(_read_number(c - '0'));
		if(c == '-' && isdigit(_peek())) return new_int(-_read_number(0));
		if(isalpha(c) || strchr(symbol_chars, c)) return _read_symbol(c);
		error("Don't know how to handle %c", c);
	}
}

Object*
next_expr(void)
{
	Object* obj = _read_expr();
	if(obj == Cparen)
		error("Stray close parenthesis");
	return obj;
}
