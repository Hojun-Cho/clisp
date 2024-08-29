#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>

void
panic(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(1);
}

void
error(char *fmt, ...)
{
	extern jmp_buf err;
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "ERROR => ");
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	longjmp(err, 1);
	exit(1);
}
