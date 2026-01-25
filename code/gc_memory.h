/* date = September 4th 2025 4:52 pm */

/*
Update notes:
2025-10-15:
expose Gcmem_ functions, aka forward declare them
2026-01-25
fix windows compilation
*/

#ifndef GC_MEMORY_H
#define GC_MEMORY_H

#include <stdint.h>
#include <string.h>

#ifndef gcmem_bool
#include <stdbool.h>
typedef bool gcmem_bool;
#endif

#ifndef gcmem_func
#define gcmem_func static
#endif

#ifndef GCMEM_ASSERT
#define GCMEM_ASSERT(expression) do {if (!(expression)) { *(volatile int *)0 = 0; }} while(0)
#endif // GCMEM_ASSERT


typedef struct Arena {
	void *data;
	int64_t allocPos;
	int64_t maxAllocPos; // keep track of maximum allocPos, for zeroing.
	int64_t bytes;
	int64_t reservedBytes;
	int64_t committedBytes;
} Arena;

typedef struct ArenaTemp {
	Arena *arena;
	int64_t originalAllocPos;
} ArenaTemp;

typedef struct PoolFreeNode PoolFreeNode;
struct PoolFreeNode {
	PoolFreeNode *next;
};

typedef struct Pool {
	uint8_t *buffer;
	int64_t bufferBytes;
	int64_t chunkSize;
	PoolFreeNode *head;
} Pool;

gcmem_func void *Gcmem_MemReserve(int64_t bytes);
gcmem_func void Gcmem_MemCommit(void *address, int64_t bytes);
gcmem_func void Gcmem_MemDecommit(void *address, int64_t bytes);
gcmem_func void Gcmem_MemFree(void *address, int64_t bytes);
gcmem_func int64_t Gcmem_GetPageSize(void);
gcmem_func void Gcmem_MemSetToZero(void *destination, int64_t bytes);

gcmem_func Arena ArenaCreate(int64_t bytes);
gcmem_func void *ArenaAlloc(Arena *arena, int64_t bytes);
gcmem_func void ArenaReset(Arena *arena);
gcmem_func void ArenaResetTo(Arena *arena, int64_t pos);
gcmem_func void ArenaFree(Arena *arena);

gcmem_func ArenaTemp ArenaBeginTemp(Arena *arena);
gcmem_func void ArenaEndTemp(ArenaTemp temp);


gcmem_func Pool PoolCreate(int64_t maxElements, int64_t chunkSize);
gcmem_func void *PoolAlloc(Pool *pool);
gcmem_func void *PoolAllocBytes(Pool *pool, int64_t bytes);
gcmem_func void PoolFreeAllElements(Pool *pool);
gcmem_func bool PoolElementFree(Pool *pool, void *memory);
gcmem_func void PoolFree(Pool *pool);

#endif //GC_MEMORY_H

#ifdef GC_MEMORY_IMPLEMENTATION

#define GCMEM_MIN(a, b) ((a) > (b) ? (b) : (a))
#define GCMEM_MAX(a, b) ((a) < (b) ? (b) : (a))

#ifdef __unix__
#include <sys/mman.h>
#include <unistd.h>

gcmem_func void *Gcmem_MemReserve(int64_t bytes)
{
	void *result = mmap(NULL, bytes, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (result == MAP_FAILED)
	{
		result = NULL;
	}
	return result;
}

gcmem_func void Gcmem_MemCommit(void *address, int64_t bytes)
{
	mprotect(address, bytes, PROT_READ | PROT_WRITE);
}

gcmem_func void Gcmem_MemDecommit(void *address, int64_t bytes)
{
	madvise(address, bytes, MADV_DONTNEED);
	mprotect(address, bytes, PROT_NONE);
}

gcmem_func void Gcmem_MemFree(void *address, int64_t bytes)
{
	munmap(address, bytes);
}

gcmem_func int64_t Gcmem_GetPageSize(void)
{
	int64_t result = sysconf(_SC_PAGESIZE);
	return result;
}

gcmem_func void Gcmem_MemSetToZero(void *destination, int64_t bytes)
{
	memset((uint8_t *)destination, 0, bytes);
}

#elifdef _WIN32

#include <windows.h>

gcmem_func void *Gcmem_MemReserve(int64_t bytes)
{
	void *result = VirtualAlloc(NULL, bytes, MEM_RESERVE, PAGE_READWRITE);
	return result;
}

gcmem_func void Gcmem_MemCommit(void *address, int64_t bytes)
{
	VirtualAlloc(address, bytes, MEM_COMMIT, PAGE_READWRITE);
}

gcmem_func void Gcmem_MemDecommit(void *address, int64_t bytes)
{
	VirtualFree(address, bytes, MEM_DECOMMIT);
}

gcmem_func void Gcmem_MemFree(void *address, int64_t bytes)
{
	VirtualFree(address, bytes, MEM_RELEASE);
}

gcmem_func int64_t Gcmem_GetPageSize(void)
{
	SYSTEM_INFO systemInfo = {0};
	GetSystemInfo(&systemInfo);
	int64_t result = (int64_t)systemInfo.dwPageSize;
	return result;
}

gcmem_func void Gcmem_MemSetToZero(void *destination, int64_t bytes)
{
	__stosb((uint8_t *)destination, 0, bytes);
}

#else
#error "Platform not supported"
#endif

gcmem_func inline int64_t Gcmem_AlignUp_(int64_t val, int64_t granularity)
{
    int64_t result = (val / granularity) * granularity;
	// NOTE(GameChaos): don't "round" 2 to 4 with granularity 2. "round" it to 2.
	if (result != val)
	{
		result += granularity;
	}
	return result;
}

gcmem_func inline int64_t Gcmem_AlignDown_(int64_t val, int64_t granularity)
{
    int64_t result = (val / granularity) * granularity;
	return result;
}

// TODO: test if this works!
gcmem_func void DecommitConservative(void *base, int64_t bytes)
{
	uintptr_t from = (uintptr_t)base;
	uintptr_t to = from + bytes;
	
	from = Gcmem_AlignUp_(from, Gcmem_GetPageSize());
	to = Gcmem_AlignDown_(to, Gcmem_GetPageSize());
	
	if (to > from)
	{
		Gcmem_MemDecommit((void *)from, to - from);
	}
}

gcmem_func Arena ArenaCreate(int64_t bytes)
{
	Arena result = {0};
	
	GCMEM_ASSERT(bytes);
	result.bytes = bytes;
	result.reservedBytes = Gcmem_AlignUp_(bytes, Gcmem_GetPageSize());
	result.data = Gcmem_MemReserve(result.reservedBytes);
	GCMEM_ASSERT(result.data);
	
	return result;
}

gcmem_func void *ArenaAlloc(Arena *arena, int64_t bytes)
{
	void *result = NULL;
	GCMEM_ASSERT(bytes <= arena->bytes - arena->allocPos);
	GCMEM_ASSERT(arena);
	if (bytes <= arena->bytes - arena->allocPos)
	{
		int64_t nextCommittedBytes = Gcmem_AlignUp_(arena->allocPos + bytes, Gcmem_GetPageSize());
		if (nextCommittedBytes > arena->committedBytes)
		{
			Gcmem_MemCommit((uint8_t *)arena->data + arena->committedBytes, nextCommittedBytes - arena->committedBytes);
			arena->committedBytes = nextCommittedBytes;
		}
		result = (uint8_t *)arena->data + arena->allocPos;
		// NOTE(GameChaos): guarantee zeroed allocation
		if (arena->allocPos < arena->maxAllocPos)
		{
			Gcmem_MemSetToZero(result, GCMEM_MIN(bytes, arena->maxAllocPos - arena->allocPos));
		}
		arena->allocPos += bytes;
		// NOTE(GameChaos): Align allocPos to 8 bytes after allocation.
		arena->allocPos = Gcmem_AlignUp_(arena->allocPos, 8);
		arena->maxAllocPos = GCMEM_MAX(arena->maxAllocPos, arena->allocPos);
	}
	
	return result;
}

gcmem_func void ArenaReset(Arena *arena)
{
	GCMEM_ASSERT(arena->data);
	GCMEM_ASSERT(arena->bytes);
	Gcmem_MemDecommit(arena->data, arena->committedBytes);
	arena->allocPos = 0;
	arena->committedBytes = 0;
	arena->maxAllocPos = 0;
}

gcmem_func void ArenaResetTo(Arena *arena, int64_t pos)
{
	if (pos < arena->allocPos)
	{
		int64_t decommitStart = Gcmem_AlignUp_(pos, Gcmem_GetPageSize());
		int64_t decommitEnd = arena->reservedBytes;
		if (decommitStart < decommitEnd)
		{
			Gcmem_MemDecommit((uint8_t *)arena->data + decommitStart,
							 decommitEnd - decommitStart);
			arena->committedBytes = decommitStart;
			arena->maxAllocPos = GCMEM_MIN(arena->maxAllocPos, decommitStart);
		}
		
		arena->allocPos = pos;
	}
}

gcmem_func void ArenaFree(Arena *arena)
{
	GCMEM_ASSERT(arena->data);
	GCMEM_ASSERT(arena->bytes);
	Gcmem_MemFree(arena->data, arena->bytes);
	arena->data = 0;
	arena->allocPos = 0;
	arena->bytes = 0;
}

gcmem_func ArenaTemp ArenaBeginTemp(Arena *arena)
{
	GCMEM_ASSERT(arena);
	GCMEM_ASSERT(arena->data);
	GCMEM_ASSERT(arena->bytes);
	ArenaTemp result = {
		arena,
		arena->allocPos
	};
	
	return result;
}

gcmem_func void ArenaEndTemp(ArenaTemp temp)
{
	ArenaResetTo(temp.arena, temp.originalAllocPos);
}

gcmem_func Pool PoolCreate(int64_t maxElements, int64_t chunkSize)
{
	Pool result = {0};
	if (maxElements && chunkSize)
	{
		int64_t bufferBytes = maxElements * chunkSize;
		GCMEM_ASSERT(chunkSize > (int64_t)sizeof(PoolFreeNode));
		GCMEM_ASSERT(bufferBytes >= chunkSize);
		result.buffer = (uint8_t *)Gcmem_MemReserve(bufferBytes);
		if (result.buffer)
		{
			result.bufferBytes = bufferBytes;
			result.chunkSize = chunkSize;
			result.head = NULL;
			
			PoolFreeAllElements(&result);
		}
		else
		{
			GCMEM_ASSERT(0);
		}
	}
	
	return result;
}

gcmem_func void *PoolAlloc(Pool *pool)
{
	void *result = PoolAllocBytes(pool, pool->chunkSize);
	return result;
}

gcmem_func void *PoolAllocBytes(Pool *pool, int64_t bytes)
{
	void *result = NULL;
	PoolFreeNode *node = pool->head;
	if (bytes == 0)
	{
		bytes = pool->chunkSize;
	}
	
	GCMEM_ASSERT(bytes <= pool->chunkSize);
	if (node != NULL && bytes <= pool->chunkSize)
	{
		pool->head = pool->head->next;
		Gcmem_MemCommit(node, bytes);
		Gcmem_MemSetToZero(node, pool->chunkSize);
		result = node;
	}
	
	return result;
}

gcmem_func void PoolFreeAllElements(Pool *pool)
{
	int64_t chunks = pool->bufferBytes / pool->chunkSize;
	
	for (int64_t i = 0; i < chunks; i++)
	{
		PoolFreeNode *node = (PoolFreeNode *)&pool->buffer[i * pool->chunkSize];
		// NOTE(GameChaos): make sure memory is committed
		Gcmem_MemCommit(node, sizeof(*node));
		node->next = pool->head;
		pool->head = node;
		DecommitConservative(node + sizeof(*node), pool->chunkSize - sizeof(*node));
	}
}

gcmem_func gcmem_bool PoolElementFree(Pool *pool, void *memory)
{
	gcmem_bool result = false;
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

gcmem_func void PoolFree(Pool *pool)
{
	Gcmem_MemFree(pool->buffer, pool->bufferBytes);
	pool->buffer = NULL;
	pool->bufferBytes = 0;
	pool->chunkSize = 0;
	pool->head = NULL;
}
#undef GCMEM_MIN
#undef GCMEM_MAX
#endif
