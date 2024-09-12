#include "dat.h"
#include "fn.h"

Object Nil	= (Object){.type=OSYMBOL, .beg="nil"};
Object Top	= (Object){.type=OSYMBOL, .beg="top"};
Object Splice= (Object){.type=OSYMBOL, .beg="@"};
Object Comma= (Object){.type=OSYMBOL, .beg=","};
Object Minus= (Object){.type=OBLTIN, .beg="-"};
Object Plus	= (Object){.type=OBLTIN, .beg="+"};
Object Mul	= (Object){.type=OBLTIN, .beg="*"};
Object Div	= (Object){.type=OBLTIN, .beg="/"};
Object Mod	= (Object){.type=OBLTIN, .beg="%"};

Object Ge	= (Object){.type=OBLTIN, .beg= ">="};
Object Le	= (Object){.type=OBLTIN, .beg= "<="};
Object Lt	= (Object){.type=OBLTIN, .beg= "<"};
Object Gt	= (Object){.type=OBLTIN, .beg= ">"};
Object Ne	= (Object){.type=OBLTIN, .beg= "!="};
Object Eq	= (Object){.type=OBLTIN, .beg= "=="};
Object Not	= (Object){.type=OBLTIN, .beg= "not"};

Object Bquote= (Object){.type=OBLTIN, .beg="`"};
Object Lambda= (Object){.type=OBLTIN, .beg="lambda"};
Object Block= (Object){.type=OBLTIN, .beg="block"};
Object RetFrom = (Object){.type=OBLTIN, .beg="return-from"};
Object Let= (Object){.type=OBLTIN, .beg="let"};
Object Progn=(Object){.type=OBLTIN, .beg="progn"};
Object Car	= (Object){.type=OBLTIN, .beg="car"};
Object Cdr	= (Object){.type=OBLTIN, .beg="cdr"};
Object Quote= (Object){.type=OBLTIN, .beg="'"};
Object Cons	= (Object){.type=OBLTIN, .beg="cons"};
Object Define= (Object){.type=OBLTIN, .beg="define"};
Object Macro= (Object){.type=OBLTIN, .beg="defmacro"};
Object Setq	= (Object){.type=OBLTIN, .beg="setq"};
Object If	= (Object){.type=OBLTIN, .beg="if"};

extern Object* fnplus(Object *, Object *);
extern Object* fnmul(Object *, Object *);
extern Object* fndiv(Object *, Object *);
extern Object* fnmod(Object *, Object *);
extern Object* fnlambda(Object *, Object *);
extern Object* fnlet(Object *, Object *);
extern Object* fnprogn(Object *, Object *);
extern Object* fnblock(Object *, Object *);
extern Object* fnretfrom(Object *, Object *);
extern Object* fndefine(Object *, Object *);
extern Object* fndefmacro(Object *, Object *);
extern Object* fnsetq(Object *, Object *);
extern Object* fnundef(Object *, Object *);
extern Object* fnquote(Object *, Object *);
extern Object* fnbquote(Object *, Object *);
extern Object* fncar(Object *, Object *);
extern Object* fncdr(Object *, Object *);
extern Object* fncons(Object *, Object *);
extern Object* fneq(Object *, Object *);
extern Object* fnnot(Object *, Object *);
extern Object* fnif(Object *, Object *);
extern Object* fnge(Object *env, Object *list);
extern Object* fngt(Object *env, Object *list);
extern Object* fnle(Object *env, Object *list);
extern Object* fnlt(Object *env, Object *list);
extern Object* fnne(Object *env, Object *list);
/*extern Object* fnminus(Object *, Object *);*/

Bltinfn
bltinlookup(Object *obj)
{
	static struct
	{
		Object *beg;
		Bltinfn fn;
	}bltins[] = {
		{&Lambda , fnlambda},
		{&Progn , fnprogn},
		{&Block , fnblock},
		{&RetFrom ,fnretfrom},
		{&Plus , fnplus},
		{&Mul , fnmul},
		{&Mod , fnmod},
		{&Div , fndiv},
		{&Define ,fndefine},
		{&Macro ,fndefmacro},
		{&Setq ,fnsetq},
		{&Let ,fnlet},
		{&Quote ,fnquote},
		{&Bquote,  fnbquote},
		{&Car ,fncar},
		{&Cdr ,fncdr},
		{&Cons ,fncons},
        {&Eq, fneq},
        {&Not, fnnot},
        {&Ne, fnne},
        {&If, fnif},
        {&Ge, fnge},
        {&Le, fnle},
        {&Gt, fngt},
        {&Lt, fnlt},
		{&Minus ,0},
        {0},
	};

	for(int i = 0; bltins[i].beg; ++i){
		if(obj == bltins[i].beg)
			return bltins[i].fn;
	}
	return 0;
}
