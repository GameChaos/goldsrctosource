/* date = November 21st 2022 1:14 pm */

#ifdef HASHMAP_KEY_TYPE
#ifdef HASHMAP_VALUE_TYPE
#ifdef HASHMAP_NAME

#define HASHMAP_PREFIX(name) TOKENPASTE(HASHMAP_NAME, name)

#define HASHMAP HASHMAP_PREFIX(Hashmap)
#define HASHMAP_PAIR HASHMAP_PREFIX(HashmapPair)
#define HASHMAP_ITERATOR HASHMAP_PREFIX(HashmapIter)

/*

#define HASHMAP_KEY_TYPE v2i
#define HASHMAP_VALUE_TYPE v2i
#define HASHMAP_NAME Tile
#include "gc_hashmap_generator.h"

api:

... is the name defined by HASHMAP_NAME

internal ...Hashmap ...HashmapCreate(Arena *arena, u64 cap)
internal bool ...HashmapPush(HASHMAP *map, HASHMAP_KEY_TYPE key, HASHMAP_VALUE_TYPE value)
internal bool ...HashmapKeyExists(HASHMAP *map, HASHMAP_KEY_TYPE key)
internal bool ...HashmapGet(HASHMAP *map, HASHMAP_KEY_TYPE key, HASHMAP_VALUE_TYPE *out)
internal bool ...HashmapPop(HASHMAP *map, HASHMAP_PAIR *out)
internal bool ...HashmapNext(HASHMAP *map, HASHMAP_ITERATOR *iter, HASHMAP_PAIR *out)

these public types will be defined:
...Hashmap;
...HashmapPair;
...HashmapIter;

sample usage:

----

TileHashmapPair pair = {0};
TileHashmapIter iter = {0};
while (TileHashmapNext(map, &iter, &pair))
{

}

*/

typedef struct HASHMAP_PAIR
{
	HASHMAP_KEY_TYPE key;
	HASHMAP_VALUE_TYPE value;
} HASHMAP_PAIR;

// NOTE(GameChaos): internal usage!
#define HASHMAP_NODE HASHMAP_PREFIX(HashmapNode_)
typedef struct HASHMAP_NODE HASHMAP_NODE;
struct HASHMAP_NODE
{
	bool valid;
	HASHMAP_PAIR pair;
	HASHMAP_NODE *next;
};

typedef struct HASHMAP
{
	HASHMAP_NODE *nodes;
	HASHMAP_NODE *first;
	HASHMAP_NODE *last;
	u64 cap; // always a power of 2
	u64 length;
} HASHMAP;

typedef struct HASHMAP_ITERATOR
{
	HASHMAP_NODE *current_;
} HASHMAP_ITERATOR;

// cap must be power of 2!
#define GCHM_POLICY(n, cap) ((n) & ((cap) - 1))

#ifndef GCHM_MURMURHASH_INCLUDED
#define GCHM_MURMURHASH_INCLUDED

// from https://github.com/jwerle/murmurhash.c
/*
The MIT License (MIT)

Copyright (c) 2014 Joseph Werle

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
static uint32_t Gchm_MurmurHash_(const char *key, uint32_t len)
{
	const uint32_t seed = 0xdeface;
	
	uint32_t c1 = 0xcc9e2d51;
	uint32_t c2 = 0x1b873593;
	uint32_t r1 = 15;
	uint32_t r2 = 13;
	uint32_t m = 5;
	uint32_t n = 0xe6546b64;
	uint32_t h = 0;
	uint32_t k = 0;
	uint8_t *d = (uint8_t *)key; // 32 bit extract from `key'
	const uint32_t *chunks = NULL;
	const uint8_t *tail = NULL; // tail - last 8 bytes
	int i = 0;
	int l = len / 4; // chunk length
	
	h = seed;
	
	chunks = (const uint32_t *)(d + l * 4); // body
	tail = (const uint8_t *)(d + l * 4); // last 8 byte chunk of `key'
	
	// for each 4 byte chunk of `key'
	for (i = -l; i != 0; ++i)
	{
		// next 4 byte chunk of `key'
		k = chunks[i];
		
		// encode next 4 byte chunk of `key'
		k *= c1;
		k = (k << r1) | (k >> (32 - r1));
		k *= c2;
		
		// append to hash
		h ^= k;
		h = (h << r2) | (h >> (32 - r2));
		h = h * m + n;
	}
	
	k = 0;
	
	// remainder
	switch (len & 3)
	{ // `len % 4'
		case 3: k ^= (tail[2] << 16);
		case 2: k ^= (tail[1] << 8);
		
		case 1:
		k ^= tail[0];
		k *= c1;
		k = (k << r1) | (k >> (32 - r1));
		k *= c2;
		h ^= k;
	}
	
	h ^= len;
	
	h ^= (h >> 16);
	h *= 0x85ebca6b;
	h ^= (h >> 13);
	h *= 0xc2b2ae35;
	h ^= (h >> 16);
	
	return h;
}
#endif // GCHM_MURMURHASH_INCLUDED

static HASHMAP HASHMAP_PREFIX(HashmapCreate)(Arena *arena, u64 cap)
{
	HASHMAP result = {0};
	
	if (cap)
	{
		cap = NextPowerOf2(cap);
		result.nodes = ArenaAlloc(arena, cap * sizeof(HASHMAP_NODE));
		if (result.nodes)
		{
			result.cap = cap;
		}
	}
	
	return result;
}

static bool HASHMAP_PREFIX(HashmapPush)(HASHMAP *map, HASHMAP_PAIR pair)
{
	bool result = false;
	
	u64 index = GCHM_POLICY(Gchm_MurmurHash_(&pair.key, sizeof(pair.key)), map->cap);
	u64 iterations = 0;
	while (map->nodes[index].valid)
	{
		iterations++;
		if (!Plat_MemCompare(&map->nodes[index].pair.key, &pair.key, sizeof(pair.key)))
		{
			// linear probe!
			index++;
			// NOTE(GameChaos): clamp
			index = GCHM_POLICY(index, map->cap);
		}
		else
		{
			return result;
		}
		if (iterations >= map->cap)
		{
			return result;
		}
	}
	HASHMAP_NODE *node = &map->nodes[index];
	map->length++;
	
	node->valid = true;
	node->pair = pair;
	node->next = NULL;
	if (map->first == NULL)
	{
		map->first = node;
	}
	if (map->last != NULL)
	{
		map->last->next = node;
	}
	map->last = node;
	result = true;
	return result;
}

static bool HASHMAP_PREFIX(HashmapKeyExists)(HASHMAP *map, HASHMAP_KEY_TYPE key)
{
	bool result = false;
	
	u64 index = GCHM_POLICY(Gchm_MurmurHash_(&key, sizeof(key)), map->cap);
	u64 iterations = 0;
	while (map->nodes[index].valid)
	{
		iterations++;
		if (Plat_MemCompare(&map->nodes[index].pair.key, &key, sizeof(key)))
		{
			result = true;
			break;
		}
		else
		{
			// linear probe!
			index++;
			// NOTE(GameChaos): clamp
			index = GCHM_POLICY(index, map->cap);
		}
		if (iterations >= map->cap)
		{
			return result;
		}
	}
	return result;
}

static bool HASHMAP_PREFIX(HashmapGet)(HASHMAP *map, HASHMAP_KEY_TYPE key, HASHMAP_VALUE_TYPE *out)
{
	bool result = false;
	
	u64 index = GCHM_POLICY(Gchm_MurmurHash_(&key, sizeof(key)), map->cap);
	u64 iterations = 0;
	while (map->nodes[index].valid)
	{
		iterations++;
		if (Plat_MemCompare(&map->nodes[index].pair.key, &key, sizeof(key)))
		{
			*out = map->nodes[index].pair.value;
			result = true;
			break;
		}
		else
		{
			// linear probe!
			index++;
			// NOTE(GameChaos): clamp
			index = GCHM_POLICY(index, map->cap);
		}
		if (iterations >= map->cap)
		{
			return result;
		}
	}
	return result;
}

static bool HASHMAP_PREFIX(HashmapPop)(HASHMAP *map, HASHMAP_PAIR *out)
{
	bool result = false;
	if (map->first)
	{
		HASHMAP_NODE *first = map->first;
		ASSERT(first->valid);
		if (first->valid)
		{
			*out = first->pair;
			result = true;
			
			first->valid = false;
			if (map->last == first)
			{
				map->last = NULL;
			}
			map->first = first->next;
			map->length--;
		}
	}
	return result;
}

static bool HASHMAP_PREFIX(HashmapNext)(HASHMAP *map, HASHMAP_ITERATOR *iter, HASHMAP_PAIR *out)
{
	bool result = false;
	
	ASSERT(map && iter && out);
	if (map && iter && out)
	{
		if (iter->current_ == NULL)
		{
			iter->current_ = map->first;
		}
		else
		{
			iter->current_ = iter->current_->next;
		}
		
		if (iter->current_)
		{
			*out = iter->current_->pair;
			result = true;
		}
	}
	
	return result;
}


#undef HASHMAP_NAME
#undef HASHMAP_VALUE_TYPE
#undef HASHMAP_KEY_TYPE
#undef HASHMAP_PREFIX
#undef HASHMAP
#undef HASHMAP_NODE
#undef GCHM_POLICY

#else
#error "Please #define HASHMAP_NAME with a name prefix!"
#endif

#else
#error "Please #define HASHMAP_VALUE_TYPE with a type!"
#endif

#else
#error "Please #define HASHMAP_KEY_TYPE with a type!"
#endif
