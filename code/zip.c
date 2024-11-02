
#include "zip.h"

internal ZipBuilder ZipBuilderCreate(Arena *arena, s64 maxBytes, s64 maxFiles)
{
	ASSERT(maxBytes > 0);
	ASSERT(maxFiles > 0);
	ASSERT(arena);
	ZipBuilder result = {};
	if (maxBytes > 0 && maxFiles > 0)
	{
		result.file = BufferCreate(arena, maxBytes);
		result.fileHeaders = (ZipFileHeader *)ArenaAlloc(arena, sizeof(*result.fileHeaders) * maxFiles);
		result.endRecord.signature = ZIP_PKID(5, 6);
		result.maxFiles = maxFiles;
		if (result.file.memory && result.file.size
			&& result.fileHeaders)
		{
			result.valid = true;
		}
	}
	
	return result;
}

internal b32 ZipBuilderBeginAddFile(ZipBuilder *builder, char *name, s32 nameLen)
{
	ASSERT(builder);
	ASSERT(builder->valid);
	ASSERT(name);
	ASSERT(!builder->addingFileCurrently);
	
	b32 result = false;
	if (name && builder && builder->valid
		&& !builder->addingFileCurrently)
	{
		if (nameLen <= 0)
		{
			nameLen = (s32)StringLength(name);
		}
		if (nameLen > 0 && nameLen < 65536)
		{
			builder->currentLocalFile = (ZipLocalFileHeader *)BufferPushSize(&builder->file,
																			 sizeof(*builder->currentLocalFile), false);
			if (builder->currentLocalFile)
			{
				builder->addingFileCurrently = true;
				builder->currentLocalFile->signature = ZIP_PKID(3, 4);
				builder->currentLocalFile->versionNeededToExtract = ZIP_VERSION_REQUIRED;
				builder->currentLocalFile->fileNameLength = (u16)nameLen;
				result = BufferPushData(&builder->file, name, nameLen, false) != NULL;
			}
		}
	}
	return result;
}

internal b32 ZipBuilderPushData(ZipBuilder *builder, void *data, s64 bytes)
{
	ASSERT(builder);
	ASSERT(builder->valid);
	ASSERT(builder->addingFileCurrently);
	ASSERT(builder->file.memory);
	ASSERT(builder->file.size);
	
	b32 result = false;
	if (builder && builder->valid && builder->addingFileCurrently)
	{
		if (BufferPushData(&builder->file, data, bytes, false))
		{
			result = true;
		}
	}
	return result;
}

internal b32 ZipBuilderEndAddFile(ZipBuilder *builder)
{
	ASSERT(builder);
	ASSERT(builder->valid);
	ASSERT(builder->addingFileCurrently);
	ASSERT(builder->currentLocalFile);
	ASSERT(builder->file.memory);
	ASSERT(builder->file.size);
	ASSERT(builder->endRecord.fileHeaderCount < builder->maxFiles);
	
	b32 result = false;
	if (builder && builder->valid
		&& builder->addingFileCurrently
		&& builder->currentLocalFile
		&& builder->file.memory
		&& builder->file.size
		&& builder->endRecord.fileHeaderCount < builder->maxFiles)
	{
		ZipLocalFileHeader *localFile = builder->currentLocalFile;
		u8 *fileStart = ((u8 *)localFile
						 + sizeof(*localFile)
						 + localFile->fileNameLength);
		
		s64 fileOffset = (s64)(fileStart - builder->file.memory);
		localFile->uncompressedSize = (u32)(builder->file.usedBytes - fileOffset);
		localFile->compressedSize = localFile->uncompressedSize;
		// NOTE(GameChaos): source itself doesn't care if the crc is wrong,
		// but when extracted as a zip, then archive programs will complain.
		// localFile->crc32 = crc32;
		
		s64 localFileOffset = (s64)((u8 *)localFile - builder->file.memory);
		ZipEndOfCentralDirRecord *endRecord = &builder->endRecord;
		builder->fileHeaders[endRecord->fileHeaderCount] = (ZipFileHeader){};
		builder->fileHeaders[endRecord->fileHeaderCount].signature = ZIP_PKID(1, 2);
		builder->fileHeaders[endRecord->fileHeaderCount].versionMadeBy = ZIP_VERSION;
		builder->fileHeaders[endRecord->fileHeaderCount].versionNeededToExtract = ZIP_VERSION_REQUIRED;
		builder->fileHeaders[endRecord->fileHeaderCount].crc32 = localFile->crc32;
		builder->fileHeaders[endRecord->fileHeaderCount].compressedSize = localFile->uncompressedSize;
		builder->fileHeaders[endRecord->fileHeaderCount].uncompressedSize = localFile->uncompressedSize;
		builder->fileHeaders[endRecord->fileHeaderCount].fileNameLength = localFile->fileNameLength;
		builder->fileHeaders[endRecord->fileHeaderCount].relativeOffsetOfLocalHeader = (u32)localFileOffset;
		endRecord->fileHeaderCount++;
		
		builder->addingFileCurrently = false;
		builder->currentLocalFile = NULL;
		result = true;
	}
	return result;
}

internal b32 ZipBuilderAddFile(ZipBuilder *builder, char *name, s32 nameLen, void *data, s64 bytes)
{
	b32 result = false;
	if (ZipBuilderBeginAddFile(builder, name, nameLen))
	{
		if (ZipBuilderPushData(builder, data, bytes))
		{
			result = ZipBuilderEndAddFile(builder);
		}
	}
	return result;
}

internal FileWritingBuffer ZipBuilderFinish(ZipBuilder *builder)
{
	ASSERT(builder);
	ASSERT(builder->valid);
	ASSERT(builder->file.memory);
	ASSERT(builder->file.size);
	
	FileWritingBuffer result = {};
	if (builder && builder->valid
		&& builder->file.memory
		&& builder->file.size)
	{
		ZipEndOfCentralDirRecord *endRecord = &builder->endRecord;
		endRecord->fileHeadersOffset = (u32)builder->file.usedBytes;
		endRecord->fileHeaderCountTotal = endRecord->fileHeaderCount;
		FileWritingBuffer *file = &builder->file;
		for (s32 i = 0; i < endRecord->fileHeaderCount; i++)
		{
			BufferPushData(&builder->file, &builder->fileHeaders[i], sizeof(builder->fileHeaders[i]), false);
			// NOTE(GameChaos): NOT NULL TERMINATED!!!
			char *name = (char *)((u8 *)file->memory
								  + builder->fileHeaders[i].relativeOffsetOfLocalHeader) + sizeof(ZipLocalFileHeader);
			BufferPushData(&builder->file, name, builder->fileHeaders[i].fileNameLength, false);
		}
		endRecord->fileHeadersBytes = (u32)(file->usedBytes - endRecord->fileHeadersOffset);
		BufferPushData(file, endRecord, sizeof(*endRecord), false);
		result = builder->file;
		*builder = (ZipBuilder){};
	}
	return result;
}
