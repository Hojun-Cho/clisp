#include "dat.h"
#include "fn.h"
#include <stdlib.h>
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
	extern jmp_buf *errptr;
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "ERROR => ");
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	longjmp(*errptr, 1);
	exit(1);
}
