/* date = November 3rd 2024 11:30 pm */

#ifdef STRINGMAP_VALUE_TYPE
#ifdef STRINGMAP_NAME

#define STRINGMAP_PREFIX(name) TOKENPASTE(STRINGMAP_NAME, name)

#define STRINGMAP STRINGMAP_PREFIX(Stringmap)
#define STRINGMAP_PAIR STRINGMAP_PREFIX(StringmapPair)
#define STRINGMAP_ITERATOR STRINGMAP_PREFIX(StringmapIter)

/*

api:

... is the name defined by STRINGMAP_NAME

static ...Stringmap ...StringmapCreate(Arena *arena, u64 cap)
static bool ...StringmapPush(...Stringmap *map, const char *key, STRINGMAP_VALUE_TYPE value)
static bool ...StringmapKeyExists(...Stringmap *map, const char *key)
static ...StringmapPair ...StringmapGet(...Stringmap *map, const char *key)
static ...StringmapPair ...StringmapPop(...Stringmap *map)
static bool ...StringmapNext(...Stringmap *map, ...StringmapIter *iter, ...StringmapPair *out)

these public types will be defined:
...Stringmap;
...StringmapPair;
...StringmapIter;

sample usage:

#define STRINGMAP_VALUE_TYPE v2i
#define STRINGMAP_NAME Tile
#include "gc_stringmap_generator.h"

----

TileStringmapPair pair = {0};
TileStringmapIter iter = {0};
while (TileStringmapNext(map, &iter, &pair))
{

}

*/

typedef struct STRINGMAP_PAIR
{
	const char *key;
	int32_t keyLength;
	STRINGMAP_VALUE_TYPE value;
} STRINGMAP_PAIR;

// NOTE(GameChaos): static usage!
#define STRINGMAP_NODE STRINGMAP_PREFIX(StringmapNode_)
typedef struct STRINGMAP_NODE STRINGMAP_NODE;
struct STRINGMAP_NODE
{
	bool valid;
	STRINGMAP_PAIR pair;
	STRINGMAP_NODE *next;
};

typedef struct STRINGMAP
{
	STRINGMAP_NODE *nodes;
	STRINGMAP_NODE *first;
	STRINGMAP_NODE *last;
	int64_t cap; // always a power of 2
	int64_t length;
} STRINGMAP;

typedef struct STRINGMAP_ITERATOR
{
	STRINGMAP_NODE *current_;
} STRINGMAP_ITERATOR;

// cap must be power of 2!
#define GCSM_POLICY(n, cap) ((n) & ((cap) - 1))

#ifndef GCSM_MURMURHASH_INCLUDED
#define GCSM_MURMURHASH_INCLUDED

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
uint32_t Gcsm_MurmurHash_(const char *key, uint32_t len)
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
		memcpy(&k, (u8 *)&chunks[i], sizeof(k));
		
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
#endif // GCSM_MURMURHASH_INCLUDED

static STRINGMAP STRINGMAP_PREFIX(StringmapCreate)(Arena *arena, int64_t cap)
{
	STRINGMAP result = {0};
	
	if (cap)
	{
		cap = NextPowerOf2(cap);
		result.nodes = ArenaAlloc(arena, cap * sizeof(STRINGMAP_NODE));
		if (result.nodes)
		{
			result.cap = cap;
		}
	}
	
	return result;
}

static bool STRINGMAP_PREFIX(StringmapPush)(STRINGMAP *map, const char *key, STRINGMAP_VALUE_TYPE value)
{
	if (!key)
	{
		return false;
	}
	
	STRINGMAP_PAIR pair = {
		.key = key,
		.keyLength = strlen(key),
		.value = value,
	};
	
	int64_t index = GCSM_POLICY(Gcsm_MurmurHash_(pair.key, pair.keyLength), map->cap);
	int64_t iterations = 0;
	while (map->nodes[index].valid)
	{
		iterations++;
		if (map->nodes[index].pair.keyLength != pair.keyLength
			|| 0 != memcmp(map->nodes[index].pair.key, pair.key, pair.keyLength))
		{
			// linear probe!
			index++;
			// NOTE(GameChaos): clamp
			index = GCSM_POLICY(index, map->cap);
		}
		else
		{
			return false;
		}
		if (iterations >= map->cap)
		{
			return false;
		}
	}
	STRINGMAP_NODE *node = &map->nodes[index];
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
	return true;
}

static bool STRINGMAP_PREFIX(StringmapKeyExists)(STRINGMAP *map, const char *key)
{
	bool result = false;
	
	int32_t keyLength = strlen(key);
	int64_t index = GCSM_POLICY(Gcsm_MurmurHash_(key, keyLength), map->cap);
	int64_t iterations = 0;
	while (map->nodes[index].valid)
	{
		iterations++;
		if (map->nodes[index].pair.keyLength == keyLength
			&& 0 == memcmp(map->nodes[index].pair.key, key, keyLength))
		{
			result = true;
			break;
		}
		else
		{
			// linear probe!
			index++;
			// NOTE(GameChaos): clamp
			index = GCSM_POLICY(index, map->cap);
		}
		if (iterations >= map->cap)
		{
			break;
		}
	}
	return result;
}

static STRINGMAP_PAIR STRINGMAP_PREFIX(StringmapGet)(STRINGMAP *map, const char *key)
{
	STRINGMAP_PAIR result = {0};
	
	int32_t keyLength = strlen(key);
	int64_t index = GCSM_POLICY(Gcsm_MurmurHash_(key, keyLength), map->cap);
	int64_t iterations = 0;
	while (map->nodes[index].valid)
	{
		iterations++;
		if (map->nodes[index].pair.keyLength == keyLength
			&& 0 == memcmp(map->nodes[index].pair.key, key, keyLength))
		{
			result = map->nodes[index].pair;
			break;
		}
		else
		{
			// linear probe!
			index++;
			// NOTE(GameChaos): clamp
			index = GCSM_POLICY(index, map->cap);
		}
		if (iterations >= map->cap)
		{
			break;
		}
	}
	return result;
}

static bool STRINGMAP_PREFIX(StringmapPop)(STRINGMAP *map, STRINGMAP_PAIR *out)
{
	bool result = false;
	
	if (map->first)
	{
		STRINGMAP_NODE *first = map->first;
		ASSERT(first->valid);
		if (first->valid)
		{
			result = true;
			*out = first->pair;
			
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

static bool STRINGMAP_PREFIX(StringmapNext)(STRINGMAP *map, STRINGMAP_ITERATOR *iter, STRINGMAP_PAIR *out)
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


#undef STRINGMAP_NAME
#undef STRINGMAP_VALUE_TYPE
#undef STRINGMAP_KEY_TYPE
#undef STRINGMAP_PREFIX
#undef STRINGMAP
#undef STRINGMAP_NODE
#undef GCSM_POLICY

#else
#error "Please #define STRINGMAP_NAME with a name prefix!"
#endif // STRINGMAP_NAME

#else
#error "Please #define STRINGMAP_VALUE_TYPE with a type!"
#endif // STRINGMAP_VALUE_TYPE
