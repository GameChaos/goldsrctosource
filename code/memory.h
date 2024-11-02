/* date = November 1st 2024 7:21 pm */

#ifndef MEMORY_H
#define MEMORY_H

typedef struct Arena {
	void *data;
	i64 allocPos;
	i64 maxAllocPos; // keep track of maximum allocPos, for zeroing.
	i64 bytes;
} Arena;

typedef struct ArenaTemp{
	Arena *arena;
	i64 originalAllocPos;
} ArenaTemp;

typedef struct PoolFreeNode PoolFreeNode;
struct PoolFreeNode {
	PoolFreeNode *next;
};

typedef struct Pool {
	u8 *buffer;
	i64 bufferBytes;
	i64 chunkSize;
	PoolFreeNode *head;
} Pool;

static_function Arena ArenaCreate(i64 bytes);
static_function void *ArenaAlloc(Arena *arena, i64 bytes);
static_function void ArenaReset(Arena *arena);
static_function void ArenaResetTo(Arena *arena, i64 pos);
static_function void ArenaFree(Arena *arena);

static_function ArenaTemp ArenaBeginTemp(Arena *arena);
static_function void ArenaEndTemp(ArenaTemp temp);


static_function Pool PoolCreate(i64 maxElements, i64 chunkSize);
static_function void *PoolAlloc(Pool *pool);
static_function void *PoolAllocBytes(Pool *pool, i64 bytes);
static_function void PoolFreeAllElements(Pool *pool);
static_function bool PoolElementFree(Pool *pool, void *memory);
static_function void PoolFree(Pool *pool);

#endif //MEMORY_H
