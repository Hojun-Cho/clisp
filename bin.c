#include "dat.h"
#include "fn.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct Bin
{
	int total;
	int using;
	int size;
	uintptr_t beg;
	uintptr_t end;
	Bin *next;
};

static int
_memrchr(uint8_t *ptr, char c, int n)
{
	while(n){
		if(ptr[n--])
			return 0;
	}
	return 1;
}

static void*
_alloc(Bin *bin)
{
	uintptr_t ptr = bin->end - 1;
	while(ptr >= bin->beg){
		if(_memrchr((void*)ptr, 0, bin->size)){
			bin->using++;
			uint8_t *res = (uint8_t*)ptr;
			res -= bin->size;
			res[0] = 1;
			return &res[1];
		}
		ptr -= bin->size - 1;
	}
	panic("panic");
}

void
bin_free(Bin *bin, void *ptr_)
{
	uintptr_t ptr = (uintptr_t)ptr_;
	while(bin){
		if(bin->beg <= ptr && ptr < bin->end){
			ptr -= 1; /* for flag */
			memset((void*)ptr, 0, bin->size);
			bin->using--;
			return;
		}
		bin = bin->next;
	}
}

void*
bin_alloc(Bin *bin)
{
	Bin *last = bin;
	while(bin){
		if(bin->using < bin->total){
			return _alloc(bin);	
		}
		last = bin;
		bin = bin->next;
	}
	bin = last->next = new_bin(bin->size, bin->total);
	return _alloc(bin);
}


Bin*
new_bin(int size, int n)
{
	Bin *bin = xalloc(sizeof(Bin));
	bin->total = n;
	bin->size = size + 1;
	bin->beg = (uintptr_t)xalloc(size * n);
	bin->end = bin->beg + bin->total * bin->size;
	return bin;
}
