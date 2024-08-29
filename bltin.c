#include "dat.h"
#include "fn.h"

Object Nil	= (Object){.type=OSYMBOL, .sym="nil"};
Object True	= (Object){.type=OSYMBOL, .sym="true"};
Object False= (Object){.type=OSYMBOL, .sym="false"};
Object Minus= (Object){.type=OBLTIN, .sym="-"};
Object Plus	= (Object){.type=OBLTIN, .sym="+"};
Object Lambda= (Object){.type=OBLTIN, .sym="lambda"};
Object Car	= (Object){.type=OBLTIN, .sym="car"};
Object Cdr	= (Object){.type=OBLTIN, .sym="cdr"};
Object Quote= (Object){.type=OBLTIN, .sym="'"};
Object Cons	= (Object){.type=OBLTIN, .sym="cons"};
Object Define= (Object){.type=OBLTIN, .sym="define"};
Object Setq	= (Object){.type=OBLTIN, .sym="setq"};

extern Object* fnplus(Object *, Object *);
extern Object* fnlambda(Object *, Object *);
extern Object* fndefine(Object *, Object *);
extern Object* fnsetq(Object *, Object *);
extern Object* fnundef(Object *, Object *);
/*extern Object* fnminus(Object *, Object *);*/
/*extern Object* fncons(Object *, Object *);*/
/*extern Object* fnquote(Object *, Object *);*/
/*extern Object* fncar(Object *, Object *);*/
/*extern Object* fncdr(Object *, Object *);*/

Bltinfn
bltinlookup(Object *obj)
{
	static struct
	{
		Object *sym;
		Bltinfn fn;
	}bltins[] = {
		{&Lambda , fnlambda},
		{&Plus , fnplus},
		{&Define ,fndefine},
		{&Setq ,fnsetq},
		{&Minus ,0},
		{&Cons ,0},
		{&Quote ,0},
		{&Car ,0},
		{&Cdr ,0},
		0,
	};

	for(int i = 0; bltins[i].sym; ++i){
		if(obj == bltins[i].sym)
			return bltins[i].fn;
	}
	return 0;
}
