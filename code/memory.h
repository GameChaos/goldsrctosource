/* date = November 1st 2024 7:21 pm */

#ifndef MEMORY_H
#define MEMORY_H

typedef struct Arena {
	void *data;
	s64 allocPos;
	s64 maxAllocPos; // keep track of maximum allocPos, for zeroing.
	s64 bytes;
} Arena;

typedef struct ArenaTemp{
	Arena *arena;
	s64 originalAllocPos;
} ArenaTemp;

typedef struct PoolFreeNode PoolFreeNode;
struct PoolFreeNode {
	PoolFreeNode *next;
};

typedef struct Pool {
	u8 *buffer;
	s64 bufferBytes;
	s64 chunkSize;
	PoolFreeNode *head;
} Pool;

internal Arena ArenaCreate(s64 bytes);
internal void *ArenaAlloc(Arena *arena, s64 bytes);
internal void ArenaReset(Arena *arena);
internal void ArenaResetTo(Arena *arena, s64 pos);
internal void ArenaFree(Arena *arena);

internal ArenaTemp ArenaBeginTemp(Arena *arena);
internal void ArenaEndTemp(ArenaTemp temp);


internal Pool PoolCreate(s64 maxElements, s64 chunkSize);
internal void *PoolAlloc(Pool *pool);
internal void *PoolAllocBytes(Pool *pool, s64 bytes);
internal void PoolFreeAllElements(Pool *pool);
internal bool PoolElementFree(Pool *pool, void *memory);
internal void PoolFree(Pool *pool);

#endif //MEMORY_H
