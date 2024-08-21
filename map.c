#include "dat.h"
#include "fn.h"
#include <stdlib.h>

struct Slot
{
	void *key;
	Object *val;
	Slot *next;
};

Object*
new_map(int cap, int (*cmp)(void*,void*), int (*hash)(Object*))
{
	Object *obj = new_object(MAP);
	obj->cap = cap;
	obj->slots = xalloc(sizeof(Slot) * obj->cap);
	obj->cmp = cmp;
	obj->hash = hash;
	return obj;
}

Object*
map_get(Object *map, void *key)
{
	int h = map->hash(key);
	for(Slot *slot = map->slots[h].next; slot; slot = slot->next) 
		if(map->cmp(slot->key, key) == 0)
			return slot->val;
	return 0;
}

void
map_set(Object *map, void *key, Object *val)
{
	int h = map->hash(key);
	Slot *last = &map->slots[h];
	for(Slot *cur = last->next; cur; cur = cur->next){
		last = cur;
	}
	last = last->next = xalloc(sizeof(Slot));
	last->key = key;
	last->val = val;
	last->next = 0;
}

void
map_iterate(Object *map, void (*fn)(Object *))
{
	for(int i = 0; i < map->cap; ++i){
		for(Slot *s = map->slots[i].next; s; s = s->next){
			fn(s->val);
		}
	}
}

void
del_map(Object *map)
{
	for(int i = 0; i < map->cap; ++i){
		Slot *ptr = map->slots[i].next;
		while(ptr){
			Slot *tmp = ptr;
			ptr = ptr->next;
			free(tmp);
		}
	}
	free(map->slots);
}
