
#include "dmx.h"

// NOTE(GameChaos): Parts taken from: https://github.com/kristiker/Datamodel.NET

static_global const char *g_dmxEncodings[] = {
	"invalid",
	"binary",
};

typedef struct
{
	u8 *data;
	i64 size;
	i64 readBytes;
} ReadBuffer;

static_function ReadBuffer ReadBufferCreate(void *data, i64 size)
{
	ReadBuffer result = {};
	result.data = (u8 *)data;
	result.size = size;
	return result;
}

static_function bool ReadBufferRead(ReadBuffer *buffer, void *out, i64 outSize)
{
	bool result = false;
	ASSERT(buffer && buffer->data && out && outSize);
	if (buffer && buffer->data)
	{
		if (buffer->readBytes + outSize <= buffer->size)
		{
			Mem_Copy(buffer->data + buffer->readBytes, out, outSize);
			buffer->readBytes += outSize;
			result = true;
		}
		else
		{
			ASSERT(0);
		}
	}
	return result;
}

static_function u8 *ReadBufferGetBytes(ReadBuffer *buffer, i64 bytes)
{
	u8 *result = NULL;
	if (buffer && buffer->data)
	{
		if (buffer->readBytes + bytes <= buffer->size)
		{
			result = buffer->data + buffer->readBytes;
			buffer->readBytes += bytes;
		}
	}
	return result;
}

#define DEFINE_READINGBUFFER_READ_TYPE(functionName, type)\
static_function bool functionName(ReadBuffer *buffer, type *out)\
{\
bool result = ReadBufferRead(buffer, out, sizeof(*out));\
return result;\
}\

DEFINE_READINGBUFFER_READ_TYPE(ReadBufferReadU8, u8);
DEFINE_READINGBUFFER_READ_TYPE(ReadBufferReadI32, i32);
DEFINE_READINGBUFFER_READ_TYPE(ReadBufferReadU64, u64);
DEFINE_READINGBUFFER_READ_TYPE(ReadBufferReadV3, v3);
DEFINE_READINGBUFFER_READ_TYPE(ReadBufferReadV4, v4);
DEFINE_READINGBUFFER_READ_TYPE(ReadBufferReadMat4, mat4);
DEFINE_READINGBUFFER_READ_TYPE(ReadBufferReadGuid, Guid);

static_function char *ReadBufferGetString(ReadBuffer *buffer)
{
	char *result = NULL;
	if (buffer && buffer->data)
	{
		if (buffer->readBytes < buffer->size)
		{
			result = (char *)(buffer->data + buffer->readBytes);
			while (*(buffer->data + buffer->readBytes))
			{
				buffer->readBytes++;
				if (buffer->readBytes == buffer->size)
				{
					result = NULL;
					break;
				}
			}
			if (result)
			{
				// NOTE(GameChaos): null terminator
				buffer->readBytes++;
			}
		}
	}
	return result;
}

static_global i32 g_attrValueContainerSize[] = {
	0, // DMX_ATTR_UNKNOWN
	sizeof(DmxElementId), // DMX_ATTR_ELEMENT
	sizeof(i32), // DMX_ATTR_INT32
	sizeof(f32), // DMX_ATTR_F32
	sizeof(bool), // DMX_ATTR_BOOL
	sizeof(char *), // DMX_ATTR_STRING
	sizeof(DmxBinaryBlob), // DMX_ATTR_BINARYBLOB
	sizeof(i32), // DMX_ATTR_TIMESPAN
	sizeof(i32), // DMX_ATTR_RGBA8
	sizeof(v2), // DMX_ATTR_VECTOR2D
	sizeof(v3), // DMX_ATTR_VECTOR3D
	sizeof(v4), // DMX_ATTR_VECTOR4D
	sizeof(v3), // DMX_ATTR_QANGLE
	sizeof(v4), // DMX_ATTR_QUATERNION
	sizeof(mat4), // DMX_ATTR_MATRIX4X4
	sizeof(u64), // DMX_ATTR_UINT64
	sizeof(u8), // DMX_ATTR_BYTE
};

static_function void ReadBufferReadDmxAttributeValue_(ReadBuffer *buffer, void *out, DmxAttrType type, DmxStringTable *stringTable/* = NULL*/)
{
	switch (type)
	{
		case DMX_ATTR_UNKNOWN:
		{
			ASSERT(0);
		} break;
		case DMX_ATTR_ELEMENT:
		{
			DmxElementId *element = (DmxElementId *)out;
			ReadBufferReadI32(buffer, &element->index);
			if (element->index == -2)
			{
				ReadBufferReadGuid(buffer, &element->guid);
			}
		} break;
		
		case DMX_ATTR_INT32:
		case DMX_ATTR_F32:
		case DMX_ATTR_TIMESPAN:
		case DMX_ATTR_RGBA8:
		{
			ReadBufferReadI32(buffer, (i32 *)out);
		} break;
		
		case DMX_ATTR_BOOL:
		case DMX_ATTR_BYTE:
		{
			ReadBufferReadU8(buffer, (u8 *)out);
		} break;
		
		case DMX_ATTR_STRING:
		{
			if (stringTable)
			{
				i32 index = 0;
				if (ReadBufferReadI32(buffer, &index) && index >= 0 && index < stringTable->stringCount)
				{
					*(char **)out = stringTable->strings[index];
				}
				ASSERT(index >= 0 && index < stringTable->stringCount);
			}
			else
			{
				*(char **)out = ReadBufferGetString(buffer);
			}
		} break;
		
		case DMX_ATTR_BINARYBLOB:
		{
			DmxBinaryBlob *blob = (DmxBinaryBlob *)out;
			ReadBufferReadI32(buffer, &blob->byteCount);
			blob->bytes = ReadBufferGetBytes(buffer, blob->byteCount);
		} break;
		
		case DMX_ATTR_VECTOR2D:
		case DMX_ATTR_UINT64:
		{
			ReadBufferReadU64(buffer, (u64 *)out);
		} break;
		
		case DMX_ATTR_VECTOR3D:
		case DMX_ATTR_QANGLE:
		{
			ReadBufferReadV3(buffer, (v3 *)out);
		} break;
		
		case DMX_ATTR_VECTOR4D:
		case DMX_ATTR_QUATERNION:
		{
			ReadBufferReadV4(buffer, (v4 *)out);
		} break;
		
		case DMX_ATTR_MATRIX4X4:
		{
			ReadBufferReadMat4(buffer, (mat4 *)out);
		} break;
		
		default:
		{
			ASSERT(0);
		};
	}
}

static_function bool ReadBufferReadDmxAttribute(ReadBuffer *buffer, DmxAttribute *attr, Arena *arena, DmxStringTable *stringTable/* = NULL*/)
{
	bool result = false;
	if (stringTable)
	{
		i32 index = 0;
		if (ReadBufferReadI32(buffer, &index) && index >= 0 && index < stringTable->stringCount)
		{
			char *string = stringTable->strings[index];
			ASSERT(strlen(string) < sizeof(attr->name));
			Format(attr->name, sizeof(attr->name), "%s", string);
		}
		ASSERT(index >= 0 && index < stringTable->stringCount);
	}
	else
	{
		char *string = ReadBufferGetString(buffer);
		ASSERT(strlen(string) < sizeof(attr->name));
		Format(attr->name, sizeof(attr->name), "%s", string);
	}
	ReadBufferReadU8(buffer, (u8 *)&attr->value.type);
	if (attr->value.type <= DMX_ATTR_COUNT)
	{
		ReadBufferReadDmxAttributeValue_(buffer, &attr->value.startOfValueData, attr->value.type, stringTable);
	}
	else
	{
		DmxAttrType valueType = (DmxAttrType)(attr->value.type - DMX_ATTR_COUNT);
		ASSERT(attr->value.type <= DMX_ATTR_COUNT || attr->value.type > DMX_ATTR_COUNT * 2);
		if (attr->value.type > DMX_ATTR_COUNT)
		{
			valueType = (DmxAttrType)(valueType - DMX_ATTR_COUNT);
		}
		ASSERT(valueType <= DMX_ATTR_COUNT);
		ReadBufferReadI32(buffer, &attr->value.arrayCount);
		if (valueType <= DMX_ATTR_COUNT)
		{
			i32 containerBytes = g_attrValueContainerSize[valueType];
			attr->value.array = ArenaAlloc(arena, attr->value.arrayCount * containerBytes);
			void *array = attr->value.array;
			for (i32 i = 0; i < attr->value.arrayCount; i++)
			{
				// NOTE(GameChaos): this doesn't reference the stringtable ever
				ReadBufferReadDmxAttributeValue_(buffer, (u8 *)array + containerBytes * i, valueType, NULL);
			}
		}
	}
	
	return result;
}

static_function DmxHeader DmxReadHeader(ReadBuffer *buf)
{
	DmxHeader result = {};
	const char *header = ReadBufferGetString(buf);
	i64 len = strlen(header);
	bool success = false;
	if (header && len >= 40)
	{
		const char *encoding = header + strlen("<!-- dmx encoding ");
		if (StringEqualsLen(encoding, "binary", strlen("binary"), true))
		{
			result.encoding = DMXENCODING_BINARY;
			const char *encodingVer = encoding + strlen("binary");
			const char *format = StringToS32(encodingVer, &result.encodingVersion);
			if (format)
			{
				if (StringEqualsLen(format, " format ", strlen(" format "), true))
				{
					format = format + strlen(" format ");
					i32 formatLen = 0;
					for (const char *c = format; *c != ' ' && *c != '\t'; c++)
					{
						formatLen++;
					}
					const char *formatVersion = format + Format(result.format, sizeof(result.format), "%.*s", formatLen, format);
					success = !!StringToS32(formatVersion, &result.formatVersion);
				}
			}
		}
	}
	if (!success)
	{
		result = (DmxHeader){};
	}
	return result;
}

static_function Dmx DmxImportBinaryNew(const char *path, Arena *arena)
{
	Dmx result = {};

	ReadFileResult file = ReadEntireFile(arena, path);
	if (file.contents)
	{
		ReadBuffer buf = ReadBufferCreate(file.contents, file.size);
		DmxHeader header = DmxReadHeader(&buf);
		if (header.encoding != DMXENCODING_BINARY
			|| header.encodingVersion != DMX_ENCODING_VERSION)
		{
			return result;
		}
		
		result = DmxCreate(arena, header.format, header.formatVersion);
		
		
	}
	
	return result;
}

static_function DmxReadBinary_v9 DmxImportBinary(const char *path, Arena *arena)
{
	DmxReadBinary_v9 result = {};
	result.file = ReadEntireFile(arena, path);
	
	if (result.file.contents)
	{
		ReadBuffer buf = ReadBufferCreate(result.file.contents, result.file.size);
		result.header = ReadBufferGetString(&buf);
		
		// prefix elements and attributes
		ReadBufferReadI32(&buf, &result.prefixElementCount);
		result.prefixElements = ArenaAlloc(arena, sizeof(*result.prefixElements) * result.prefixElementCount);
		for (i32 prefixElem = 0; prefixElem < result.prefixElementCount; prefixElem++)
		{
			DmxReadElemBody *element = &result.prefixElements[prefixElem];
			ReadBufferReadI32(&buf, &element->attributeCount);
			element->attributes = ArenaAlloc(arena, sizeof(*element->attributes) * element->attributeCount);
			for (i32 attr = 0; attr < element->attributeCount; attr++)
			{
				ReadBufferReadDmxAttribute(&buf, &element->attributes[attr], arena, NULL);
			}
		}
		
		// string table
		ReadBufferReadI32(&buf, &result.stringTable.stringCount);
		result.stringTable.strings = ArenaAlloc(arena, sizeof(*result.stringTable.strings) * result.stringTable.stringCount);
		for (i32 stringIndex = 0; stringIndex < result.stringTable.stringCount; stringIndex++)
		{
			result.stringTable.strings[stringIndex] = ReadBufferGetString(&buf);
			Print("stringTable[%i]: %s\n", stringIndex, result.stringTable.strings[stringIndex]);
		}
		
		// element headers
		ReadBufferReadI32(&buf, &result.elementCount);
		result.elementHeaders = ArenaAlloc(arena, sizeof(*result.elementHeaders) * result.elementCount);
		for (i32 elem = 0; elem < result.elementCount; elem++)
		{
			DmxReadElemHeader *elemHeader = &result.elementHeaders[elem];
			i32 typeIndex = 0;
			i32 nameIndex = 0;
			ReadBufferReadI32(&buf, &typeIndex);
			ReadBufferReadI32(&buf, &nameIndex);
			ReadBufferReadGuid(&buf, &elemHeader->guid);
			elemHeader->type = result.stringTable.strings[typeIndex];
			elemHeader->name = result.stringTable.strings[nameIndex];
		}
		
		// element bodies
		result.elements = ArenaAlloc(arena, sizeof(*result.elements) * result.elementCount);
		for (i32 elem = 0; elem < result.elementCount; elem++)
		{
			DmxReadElemBody *element = &result.elements[elem];
			ReadBufferReadI32(&buf, &element->attributeCount);
			element->attributes = ArenaAlloc(arena, sizeof(*element->attributes) * element->attributeCount);
			for (i32 attr = 0; attr < element->attributeCount; attr++)
			{
				ReadBufferReadDmxAttribute(&buf, &element->attributes[attr], arena, &result.stringTable);
			}
		}
	}
	
	return result;
}

static_function void WriteDmxAttributeValue(FileWritingBuffer *buffer, DmxAttrValue *value, IntStringmap *stringtable)
{
	ASSERT_RANGE(value->type, 0, DMX_ATTR_COUNT);
	switch (value->type)
	{
		case DMX_ATTR_UNKNOWN:
		{
			ASSERT(0);
		} break;
		
		case DMX_ATTR_ELEMENT:
		{
			BufferPushI32(buffer, value->element.index, false);
			if (value->element.index == -2)
			{
				BufferPushData(buffer, &value->element.guid, sizeof(value->element.guid), false);
			}
		} break;
		
		case DMX_ATTR_INT32:
		case DMX_ATTR_F32:
		case DMX_ATTR_TIMESPAN:
		case DMX_ATTR_RGBA8:
		{
			BufferPushI32(buffer, value->int32, false);
		} break;
		
		case DMX_ATTR_BOOL:
		case DMX_ATTR_BYTE:
		{
			BufferPushU8(buffer, value->byte, false);
		} break;
		
		case DMX_ATTR_STRING:
		{
			if (stringtable)
			{
				BufferPushI32(buffer, IntStringmapGet(stringtable, value->string).value, false);
			}
			else
			{
				BufferWriteCString(buffer, value->string);
			}
		} break;
		
		case DMX_ATTR_BINARYBLOB:
		{
			BufferPushI32(buffer, value->binaryBlob.byteCount, false);
			BufferPushData(buffer, value->binaryBlob.bytes, value->binaryBlob.byteCount, false);
		} break;
		
		case DMX_ATTR_VECTOR2D:
		case DMX_ATTR_UINT64:
		{
			BufferPushU64(buffer, value->uint64, false);
		} break;
		
		case DMX_ATTR_VECTOR3D:
		case DMX_ATTR_QANGLE:
		{
			BufferPushV3(buffer, value->vector3, false);
		} break;
		
		case DMX_ATTR_VECTOR4D:
		case DMX_ATTR_QUATERNION:
		{
			BufferPushV4(buffer, value->vector4, false);
		} break;
		
		case DMX_ATTR_MATRIX4X4:
		{
			BufferPushMat4(buffer, value->matrix4x4, false);
		} break;
		
		default:
		{
			ASSERT(0);
		};
	}
}

static_function void WriteDmxAttribute_(FileWritingBuffer *buffer, DmxAttribute *attr, IntStringmap *stringtable)
{
	// TODO: convert asserts to proper errors
	if (stringtable)
	{
		BufferPushI32(buffer, IntStringmapGet(stringtable, attr->name).value, false);
	}
	else
	{
		BufferWriteCString(buffer, attr->name);
	}
	BufferPushU8(buffer, attr->value.type, false);
	if (attr->value.type <= DMX_ATTR_COUNT)
	{
		WriteDmxAttributeValue(buffer, &attr->value, stringtable);
	}
	else
	{
		DmxAttrType valueType = (DmxAttrType)(attr->value.type - DMX_ATTR_COUNT);
		ASSERT(attr->value.type <= DMX_ATTR_COUNT || attr->value.type > DMX_ATTR_COUNT * 2);
		if (attr->value.type > DMX_ATTR_COUNT)
		{
			valueType = (DmxAttrType)(valueType - DMX_ATTR_COUNT);
		}
		ASSERT(valueType <= DMX_ATTR_COUNT);
		BufferPushI32(buffer, attr->value.arrayCount, false);
		if (valueType <= DMX_ATTR_COUNT)
		{
			i32 containerBytes = g_attrValueContainerSize[valueType];
			for (i32 i = 0; i < attr->value.arrayCount; i++)
			{
				// NOTE(GameChaos): this doesn't reference the stringtable ever
				//ReadBufferReadDmxAttributeValue_(buffer, (u8 *)array + containerBytes * i, valueType, NULL);
				// TODO: make sure all the casting, offsets etc here is correct
				WriteDmxAttributeValue(buffer, (DmxAttrValue *)((u8 *)attr->value.array + containerBytes * i), NULL);
			}
		}
	}
}

static_function void DmxExportBinary(const char *path, Arena *arena, Dmx dmx)
{
	if (dmx.header.encoding != DMXENCODING_BINARY)
	{
		ASSERT(0);
		return;
	}
	
	ArenaTemp tempArena = ArenaBeginTemp(arena);
	FileWritingBuffer buffer = BufferCreate(arena, GIGABYTES(1));
	
	char header[128];
	// TODO: support other dmx types
	i32 chars = Format(header, sizeof(header), "<!-- dmx encoding %s %i format %s %i -->\n",
					   g_dmxEncodings[dmx.header.encoding], DMX_ENCODING_VERSION, dmx.header.format, 35);
	BufferPushData(&buffer, header, chars + 1, false);
	
	BufferPushI32(&buffer, dmx.prefix.count, false);
	// NOTE(GameChaos): Write prefix elements
	for (i32 elemInd = 0; elemInd < dmx.prefix.count; elemInd++)
	{
		DmxElement *elem = &dmx.prefix.elements[elemInd];
		BufferPushI32(&buffer, elem->attributeCount, false);
		
		for (i32 attrInd = 0; attrInd < elem->attributeCount; attrInd++)
		{
			DmxAttribute *attr = &elem->attributes[attrInd];
			
			WriteDmxAttribute_(&buffer, attr, NULL);
		}
	}
	
	DmxStringTable stringTable = {};
	i32 stringCount = 0;
	// construct stringtable
	for (i32 elemInd = 0; elemInd < dmx.body.count; elemInd++)
	{
		DmxElement *elem = &dmx.body.elements[elemInd];
		
		stringCount++; // elem->type
		stringCount++; // elem->name
		for (i32 attrInd = 0; attrInd < elem->attributeCount; attrInd++)
		{
			DmxAttribute *attr = &elem->attributes[attrInd];
			stringCount++; // attr->name
			if (attr->value.type == DMX_ATTR_STRING)
			{
				stringCount++;
			}
		}
	}
	
	// TODO: check what a good cap would be with very large maps.
	//  most strings should be repeated many times, so it should be
	//  quite low actually
	IntStringmap stringtable = IntStringmapCreate(arena, stringCount + 10);
	for (i32 elemInd = 0; elemInd < dmx.body.count; elemInd++)
	{
		DmxElement *elem = &dmx.body.elements[elemInd];
		
		if (!IntStringmapKeyExists(&stringtable, elem->type))
		{
			IntStringmapPush(&stringtable, elem->type, (i32)stringtable.length);
		}
		if (!IntStringmapKeyExists(&stringtable, elem->name))
		{
			IntStringmapPush(&stringtable, elem->name, (i32)stringtable.length);
		}
		for (i32 attrInd = 0; attrInd < elem->attributeCount; attrInd++)
		{
			DmxAttribute *attr = &elem->attributes[attrInd];
			if (attr->value.type == DMX_ATTR_STRING
				&& !IntStringmapKeyExists(&stringtable, attr->value.string))
			{
				IntStringmapPush(&stringtable, attr->value.string, (i32)stringtable.length);
			}
		}
	}
	
	// write stringtable
	BufferPushI32(&buffer, (i32)stringtable.length, false);
	IntStringmapPair pair = {0};
	IntStringmapIter iter = {0};
	while (IntStringmapNext(&stringtable, &iter, &pair))
	{
		Print("%s: %i\n", pair.key, pair.value);
		BufferPushData(&buffer, pair.key, strlen(pair.key) + 1, false);
	}
	
	// element headers
	BufferPushI32(&buffer, dmx.body.count, false);
	for (i32 elemInd = 0; elemInd < dmx.body.count; elemInd++)
	{
		DmxElement *elem = &dmx.body.elements[elemInd];
		
		i32 typeIndex = IntStringmapGet(&stringtable, elem->type).value;
		i32 nameIndex = IntStringmapGet(&stringtable, elem->name).value;
		BufferPushI32(&buffer, typeIndex, false);
		BufferPushI32(&buffer, nameIndex, false);
		BufferPushData(&buffer, &elem->guid, sizeof(elem->guid), false);
	}
	
	// element bodies
	for (i32 elemInd = 0; elemInd < dmx.body.count; elemInd++)
	{
		DmxElement *elem = &dmx.body.elements[elemInd];
		
		BufferPushI32(&buffer, elem->attributeCount, false);
		for (i32 attrInd = 0; attrInd < elem->attributeCount; attrInd++)
		{
			DmxAttribute *attr = &elem->attributes[attrInd];
			
			WriteDmxAttribute_(&buffer, attr, &stringtable);
		}
	}
	
	WriteEntireFile(path, buffer.memory, buffer.usedBytes);
	ArenaEndTemp(tempArena);
}

// NOTE(GameChaos): i hope source 2 doesn't care that my guids don't conform to the standard.
// TODO: Make conformant guids?
static_function Guid GenerateGuid()
{
	Guid result = {};
	for (i32 i = 0; i < 4; i++)
	{
		result.uints[i] = pcg32_random();
	}
	return result;
}

static_function Dmx DmxCreate(Arena *arena, const char *format, i32 formatVersion)
{
	Dmx result = {
		{
			.encoding = DMXENCODING_BINARY,
			.encodingVersion = DMX_ENCODING_VERSION,
			.formatVersion = formatVersion,
		},
		{
			.count = 0,
			.max = 128,
		},
		{
			.count = 0,
			.max = 1024,
		},
	};
	Format(result.header.format, sizeof(result.header.format), "%s", format);
	result.prefix.elements = ArenaAlloc(arena, result.prefix.max * sizeof(*result.prefix.elements));
	result.body.elements = ArenaAlloc(arena, result.body.max * sizeof(*result.body.elements));
	
	return result;
}

static_function DmxAttribute *DmxAddAttribute(DmxElement *parent, str name, DmxAttrType type)
{
	ASSERT(parent && name.data);
	DmxAttribute *result = NULL;
	if (parent->attributeCount < parent->maxAttributes)
	{
		result = &parent->attributes[parent->attributeCount++];
		*result = (DmxAttribute){};
		result->value.type = type;
		Format(result->name, sizeof(result->name), "%.*s", (i32)name.length, name.data);
	}
	ASSERT(result);
	
	return result;
}

static_function void DmxAttrSetData(DmxAttribute *attr, void *data, i64 bytes)
{
	ASSERT(data && bytes);
	
	switch (attr->value.type)
	{
		case DMX_ATTR_ELEMENT:
		case DMX_ATTR_INT32:
		case DMX_ATTR_F32:
		case DMX_ATTR_BOOL:
		case DMX_ATTR_TIMESPAN:
		case DMX_ATTR_RGBA8:
		case DMX_ATTR_VECTOR2D:
		case DMX_ATTR_VECTOR3D:
		case DMX_ATTR_VECTOR4D:
		case DMX_ATTR_QANGLE:
		case DMX_ATTR_QUATERNION:
		case DMX_ATTR_MATRIX4X4:
		case DMX_ATTR_UINT64:
		case DMX_ATTR_BYTE:
		{
			Mem_Copy(data, &attr->value.startOfValueData, bytes);
		} break;
		
		case DMX_ATTR_STRING:
		{
			attr->value.string = (char *)data;
		} break;
		
		case DMX_ATTR_BINARYBLOB:
		{
			attr->value.binaryBlob.bytes = (u8 *)data;
			attr->value.binaryBlob.byteCount = bytes;
		} break;
		
		default:
		{
			ASSERT(0);
		};
	}
}

#define DEFINE_DMXADDATTRIBUTE_FUNC(functionName, dataType, attrType)\
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(functionName, dataType)\
{\
    DmxAttribute *result = DmxAddAttribute(parent, name, attrType);\
    if (result)\
    {\
        DmxAttrSetData(result, &(value), (i64)sizeof(value));\
    }\
    return result;\
}\

DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeElementId, DmxElementId, DMX_ATTR_ELEMENT)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeInt, i32, DMX_ATTR_INT32)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeF32, f32, DMX_ATTR_F32)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeBool, bool, DMX_ATTR_BOOL)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeTimespan, i32, DMX_ATTR_TIMESPAN)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeRgba8, u32, DMX_ATTR_RGBA8)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeV2, v2, DMX_ATTR_VECTOR2D)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeV3, v3, DMX_ATTR_VECTOR3D)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeV4, v4, DMX_ATTR_VECTOR4D)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeQAngle, v3, DMX_ATTR_QANGLE)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeQuaternion, v4, DMX_ATTR_QUATERNION)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeMat4, mat4, DMX_ATTR_MATRIX4X4)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeU64, u64, DMX_ATTR_UINT64)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeU8, u8, DMX_ATTR_BYTE)

#undef DEFINE_DMXADDATTRIBUTE_FUNC

static_function DmxAttribute *DmxAddAttributeBinary(DmxElement *parent, str name, void *binaryBlob, i64 bytes)
{
	DmxAttribute *result = DmxAddAttribute(parent, name, DMX_ATTR_BINARYBLOB);
    if (result)
    {
        DmxAttrSetData(result, binaryBlob, bytes);
    }
    return result;
}

static_function DmxAttribute *DmxAddAttributeString(DmxElement *parent, str name, str value)
{
	DmxAttribute *result = DmxAddAttribute(parent, name, DMX_ATTR_STRING);
    if (result)
    {
        DmxAttrSetData(result, (void *)value.data, 1);
    }
    return result;
}

#define DEFINE_DMXADDATTRIBUTEARRAY_FUNC(functionName, dataType, attrType)\
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(functionName, dataType)\
{\
    DmxAttribute *result = DmxAddAttribute(parent, name, attrType);\
    if (result)\
    {\
        result->value.arrayCount = itemCount;\
        result->value.array = items;\
    }\
    return result;\
}\

DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayElementId, DmxElementId, DMX_ATTR_ELEMENT_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayInt, i32, DMX_ATTR_INT32_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayBool, bool, DMX_ATTR_BOOL_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayTimespan, i32, DMX_ATTR_TIMESPAN_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayRgba8, u32, DMX_ATTR_RGBA8_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayV2, v2, DMX_ATTR_VECTOR2D_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayV3, v3, DMX_ATTR_VECTOR3D_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayV4, v4, DMX_ATTR_VECTOR4D_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayQAngle, v3, DMX_ATTR_QANGLE_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayQuaternion, v4, DMX_ATTR_QUATERNION_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayMat4, mat4, DMX_ATTR_MATRIX4X4_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayU64, u64, DMX_ATTR_UINT64_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayU8, u8, DMX_ATTR_BYTE_ARRAY)

#undef DEFINE_DMXADDATTRIBUTEARRAY_FUNC

static_function DmxElement *DmxCreateElement_(DmxElements *elements, str name, str type, Arena *arena)
{
	ASSERT(name.data && type.data);
	DmxElement *result = NULL;
	if (elements->count < elements->max)
	{
		result = &elements->elements[elements->count++];
		*result = (DmxElement){};
		result->maxAttributes = 1024;
		result->attributes = ArenaAlloc(arena, result->maxAttributes * sizeof(*result->attributes));
		Format(result->type, sizeof(result->type), "%.*s", (i32)type.length, type.data);
		Format(result->name, sizeof(result->name), "%.*s", (i32)name.length, name.data);
		result->guid = GenerateGuid();
	}
	ASSERT(result);
	return result;
}

static_function DmxElement *DmxAddElement(DmxElements *elements, DmxElement *parent, str name, str type, Arena *arena)
{
	DmxElement *result = DmxCreateElement_(elements, name, type, arena);
	if (result)
	{
		DmxElementId id = {
			elements->count - 1,
			result->guid
		};
		if (parent)
		{
			DmxAttribute *attr = DmxAddAttributeElementId(parent, name, id);
			ASSERT(attr);
		}
	}
	ASSERT(result);
	return result;
}

static_function void DmxTest(Arena *arena, Arena *tempArena)
{
	// load test
	{
		Dmx dmxTestNew = DmxImportBinaryNew("debug/empty.vmap", arena);
		DmxReadBinary_v9 dmxTest = DmxImportBinary("debug/empty.vmap", arena);
		//DmxReadBinary_v9 dmxTest = DmxImportBinary("debug/out.vmap", arena);
		
		// reexport test
		//DmxExportBinary("debug/out.vmap", arena, dmxTest);
		
#if 1
		for (i32 elem = 0; elem < dmxTest.prefixElementCount; elem++)
		{
			DmxReadElemBody *elemBody = &dmxTest.prefixElements[elem];
			Print("{\n");
			for (i32 attrInd = 0; attrInd < elemBody->attributeCount; attrInd++)
			{
				DmxAttribute *attr = &elemBody->attributes[attrInd];
				Print("\t\"%s\" \"%i\" \"\"\n", attr->name, attr->value.type);
			}
			Print("%s", "}\n");
		}
		
		for (i32 elem = 0; elem < dmxTest.elementCount; elem++)
		{
			DmxReadElemHeader *elemHeader = &dmxTest.elementHeaders[elem];
			DmxReadElemBody *elemBody = &dmxTest.elements[elem];
			Print("\"%s\" \"%s\"\n{\n", elemHeader->type, elemHeader->name);
			for (i32 attrInd = 0; attrInd < elemBody->attributeCount; attrInd++)
			{
				DmxAttribute *attr = &elemBody->attributes[attrInd];
				Print("\t\"%s\" \"%i\" \"\"\n", attr->name, attr->value.type);
			}
			Print("%s", "}\n");
		}
#endif
		
		dmxTest = dmxTest;
	}
	
#if 1
	// write test
	{
		Dmx dmx = DmxCreate(arena, "vmap", 35);
		
		// prefix attributes
		{
			DmxElement *prefixElement = DmxAddElement(&dmx.prefix, NULL, STR(""), STR(""), arena);
			const char *imgData = "P3\n2 2\n0 0 0\n0 0 0\n0 0 0\n0 0 0";
			i64 imgBytes = strlen(imgData);
			DmxAddAttributeBinary(prefixElement, STR("asset_preview_thumbnail"), imgData, imgBytes);
			DmxAddAttributeString(prefixElement, STR("asset_preview_thumbnail_format"), STR("ppm"));
			DmxAddAttribute(prefixElement, STR("map_asset_references"), DMX_ATTR_STRING_ARRAY);
		}
		
		// CMapRootElement
		{
			DmxElement *mapRootElement = DmxAddElement(&dmx.body, NULL, STR(""), STR("CMapRootElement"), arena);
			DmxAddAttributeBool(mapRootElement, STR("isprefab"), false);
			DmxAddAttributeInt(mapRootElement, STR("editorbuild"), 9820);
			DmxAddAttributeInt(mapRootElement, STR("editorversion"), 400);
			DmxAddAttributeString(mapRootElement, STR("itemFile"), STR(""));
			
			// defaultcamera
			{
				DmxElement *defaultCamera = DmxAddElement(&dmx.body, mapRootElement, STR("defaultcamera"), STR("CStoredCamera"), arena);
				DmxAddAttributeInt(defaultCamera, STR("activecamera"), -1);
				DmxAddAttributeV3(defaultCamera, STR("position"), (v3){0, -1000, -1000});
				DmxAddAttributeV3(defaultCamera, STR("lookat"), (v3){-0.0000000618f, -999.2929077148f, 999.2929077148f});
			}
			
			// 3dcameras
			{
				DmxElement *cameras = DmxAddElement(&dmx.body, mapRootElement, STR("3dcameras"), STR("CStoredCameras"), arena);
				DmxAddAttributeArrayElementId(cameras, STR("cameras"), NULL, 0);
			}
			
			// world
			{
				DmxElement *world = DmxAddElement(&dmx.body, mapRootElement, STR("world"), STR("CMapWorld"), arena);
				// TODO: correct nodeid
				DmxAddAttributeInt(mapRootElement, STR("nodeID"), 1);
				DmxAddAttributeU64(mapRootElement, STR("referenceID"), 0);
				DmxAddAttributeArrayElementId(mapRootElement, STR("children"), NULL, 0);
				//DmxAddAttributeArrayString(mapRootElement, STR("variableTargetKeys"), NULL, 0);
				//DmxAddAttributeArrayString(mapRootElement, STR("variableNames"), NULL, 0);
			}
		}
	}
#endif
	return;
}