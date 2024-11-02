
#include "memory.h"

static_function inline i64 AlignUp_(i64 val, i64 granularity)
{
    i64 result = (val / granularity) * granularity;
	// NOTE(GameChaos): don't "round" 2 to 4 with granularity 2. "round" it to 2.
	if (result != val)
	{
		result += granularity;
	}
	return result;
}

static_function inline i64 AlignDown_(i64 val, i64 granularity)
{
    i64 result = (val / granularity) * granularity;
	return result;
}

// TODO: test if this works!
static_function void DecommitConservative(void *base, i64 bytes)
{
	uintptr_t from = (uintptr_t)base;
	uintptr_t to = from + bytes;
	
	from = AlignUp_(from, Plat_GetPageSize());
	to = AlignDown_(to, Plat_GetPageSize());
	
	if (to > from)
	{
		Plat_MemDecommit((void *)from, to - from);
	}
}

static_function Arena ArenaCreate(i64 bytes)
{
	Arena result = {0};
	
	ASSERT(bytes);
	result.data = Plat_MemReserve(bytes);
	result.bytes = bytes;
	ASSERT(result.data);
	
	return result;
}

static_function void *ArenaAlloc(Arena *arena, i64 bytes)
{
	void *result = NULL;
	ASSERT(bytes <= arena->bytes - arena->allocPos);
	ASSERT(arena);
	if (bytes <= arena->bytes - arena->allocPos)
	{
		result = (u8 *)arena->data + arena->allocPos;
		Plat_MemCommit((u8 *)arena->data + arena->allocPos, bytes);
		// NOTE(GameChaos): guarantee zeroed allocation
		if (arena->allocPos < arena->maxAllocPos)
		{
			Plat_MemSetToZero(result, GCM_MIN(bytes, arena->maxAllocPos - arena->allocPos));
		}
		arena->allocPos += bytes;
		// NOTE(GameChaos): Align allocPos to 8 bytes after allocation.
		arena->allocPos = AlignUp_(arena->allocPos, 8);
		arena->maxAllocPos = GCM_MAX(arena->maxAllocPos, arena->allocPos);
	}
	
	return result;
}

static_function void ArenaReset(Arena *arena)
{
	ASSERT(arena->data);
	ASSERT(arena->bytes);
	Plat_MemDecommit(arena->data, arena->bytes);
	arena->allocPos = 0;
}

static_function void ArenaResetTo(Arena *arena, i64 pos)
{
	if (pos < arena->allocPos)
	{
		i64 decommitStart = AlignUp_(pos, Plat_GetPageSize());
		i64 decommitEnd = arena->bytes;
		if (decommitStart < decommitEnd)
		{
			Plat_MemDecommit((u8 *)arena->data + decommitStart,
							 decommitEnd - decommitStart);
		}
		
		arena->allocPos = pos;
	}
}

static_function void ArenaFree(Arena *arena)
{
	ASSERT(arena->data);
	ASSERT(arena->bytes);
	Plat_MemFree(arena->data, arena->bytes);
	arena->data = 0;
	arena->allocPos = 0;
	arena->bytes = 0;
}

static_function ArenaTemp ArenaBeginTemp(Arena *arena)
{
	ASSERT(arena);
	ASSERT(arena->data);
	ASSERT(arena->bytes);
	ArenaTemp result = {
		arena,
		arena->allocPos
	};
	
	return result;
}

static_function void ArenaEndTemp(ArenaTemp temp)
{
	if (temp.originalAllocPos != temp.arena->allocPos)
	{
		i64 decommitStart = AlignUp_(temp.originalAllocPos, Plat_GetPageSize());
		i64 decommitEnd = AlignUp_(temp.arena->bytes, Plat_GetPageSize());
		if (decommitStart < decommitEnd)
		{
			Plat_MemDecommit((u8 *)temp.arena->data + decommitStart,
							 decommitEnd - decommitStart);
		}
		
		temp.arena->allocPos = temp.originalAllocPos;
	}
}

static_function Pool PoolCreate(i64 maxElements, i64 chunkSize)
{
	Pool result = {0};
	if (maxElements && chunkSize)
	{
		i64 bufferBytes = maxElements * chunkSize;
		ASSERT(chunkSize > sizeof(PoolFreeNode));
		ASSERT(bufferBytes >= chunkSize);
		result.buffer = (u8 *)Plat_MemReserve(bufferBytes);
		if (result.buffer)
		{
			result.bufferBytes = bufferBytes;
			result.chunkSize = chunkSize;
			result.head = NULL;
			
			PoolFreeAllElements(&result);
		}
		else
		{
			ASSERT(0);
		}
	}
	
	return result;
}

static_function void *PoolAlloc(Pool *pool)
{
	void *result = PoolAllocBytes(pool, pool->chunkSize);
	return result;
}

static_function void *PoolAllocBytes(Pool *pool, i64 bytes)
{
	void *result = NULL;
	PoolFreeNode *node = pool->head;
	if (bytes == 0)
	{
		bytes = pool->chunkSize;
	}
	
	ASSERT(bytes <= pool->chunkSize);
	if (node != NULL && bytes <= pool->chunkSize)
	{
		pool->head = pool->head->next;
		Plat_MemCommit(node, bytes);
		Plat_MemSetToZero(node, pool->chunkSize);
		result = node;
	}
	
	return result;
}

static_function void PoolFreeAllElements(Pool *pool)
{
	i64 chunks = pool->bufferBytes / pool->chunkSize;
	
	for (i64 i = 0; i < chunks; i++)
	{
		PoolFreeNode *node = (PoolFreeNode *)&pool->buffer[i * pool->chunkSize];
		// NOTE(GameChaos): make sure memory is committed
		Plat_MemCommit(node, sizeof(*node));
		node->next = pool->head;
		pool->head = node;
		DecommitConservative(node + sizeof(*node), pool->chunkSize - sizeof(*node));
	}
}

static_function bool PoolElementFree(Pool *pool, void *memory)
{
	bool result = false;
	if (memory != NULL)
	{
		void *start = pool->buffer;
		void *end = pool->buffer + pool->bufferBytes;
		
		if (start <= memory && memory < end)
		{
			PoolFreeNode *node = (PoolFreeNode *)memory;
			node->next = pool->head;
			pool->head = node;
			DecommitConservative(node + sizeof(*node), pool->chunkSize - sizeof(*node));
			result = true;
		}
	}
	return result;
}

static_function void PoolFree(Pool *pool)
{
	Plat_MemFree(pool->buffer, pool->bufferBytes);
	pool->buffer = NULL;
	pool->bufferBytes = 0;
	pool->chunkSize = 0;
	pool->head = NULL;
}
