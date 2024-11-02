/* date = November 2nd 2022 2:53 pm */

#ifndef ZIP_H
#define ZIP_H

#define ZIP_VERSION_REQUIRED 10
#define ZIP_VERSION 20
#define ZIP_PKID(a, b) (((b) << 24) | ((a) << 16) | ('K' << 8) | 'P')

#pragma pack(push, 1)
typedef struct
{
	u32 signature; // ZIP_PKID(5, 6)
	u16 diskNum;
	u16 diskStart;
	u16 fileHeaderCount;
	u16 fileHeaderCountTotal;
	u32 fileHeadersBytes;
	u32 fileHeadersOffset;
	u16 commentLength;
} ZipEndOfCentralDirRecord;

typedef struct
{
	u32 signature; // ZIP_PKID(1, 2)
	u16 versionMadeBy;
	u16 versionNeededToExtract;
	u16 flags;
	u16 compressionMethod;
	u16 lastModifiedTime;
	u16 lastModifiedDate;
	u32 crc32;
	u32 compressedSize;
	u32 uncompressedSize;
	u16 fileNameLength;
	u16 extraFieldLength;
	u16 fileCommentLength;
	u16 diskNumberStart;
	u16 internalFileAttribs;
	u32 externalFileAttribs;
	u32 relativeOffsetOfLocalHeader;
} ZipFileHeader;

typedef struct
{
	u32 signature; // ZIP_PKID(3, 4)
	u16 versionNeededToExtract;
	u16 flags;
	u16 compressionMethod;
	u16 lastModifiedTime;
	u16 lastModifiedDate;
	u32 crc32;
	u32 compressedSize;
	u32 uncompressedSize;
	u16 fileNameLength;
	u16 extraFieldLength;
} ZipLocalFileHeader;
static_assert(sizeof(ZipLocalFileHeader) == 30, "");
#pragma pack(pop)

typedef struct
{
	// used by beginaddfile/pushdata/endaddfile
	b32 addingFileCurrently;
	ZipLocalFileHeader *currentLocalFile;
	
	b32 valid;
	FileWritingBuffer file;
	s64 maxFiles;
	ZipFileHeader *fileHeaders; // NOTE(GameChaos): fileHeaders count is in endRecord
	ZipEndOfCentralDirRecord endRecord;
} ZipBuilder;

#endif //ZIP_H
