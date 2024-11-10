
#include "dmx.h"

// NOTE(GameChaos): Parts taken from: https://github.com/kristiker/Datamodel.NET

static_global const char *g_dmxEncodings[] = {
	"invalid",
	"binary",
};

static_global const char *g_dmxAttrTypes[] = {
	"unknown", //DMX_ATTR_UNKNOWN    = 0,
	"elementid", //DMX_ATTR_ELEMENT    = 1,
	"int", //DMX_ATTR_INT32      = 2,
	"float", //DMX_ATTR_F32        = 3,
	"bool", //DMX_ATTR_BOOL       = 4,
	"string", //DMX_ATTR_STRING     = 5,
	"binary", //DMX_ATTR_BINARYBLOB = 6,
	"time", //DMX_ATTR_TIMESPAN   = 7,
	"rgba8", //DMX_ATTR_RGBA8      = 8,
	"Vector2D", //DMX_ATTR_VECTOR2D   = 9,
	"Vector", //DMX_ATTR_VECTOR3D   = 10,
	"Vector4D", //DMX_ATTR_VECTOR4D   = 11,
	"QAngle", //DMX_ATTR_QANGLE     = 12,
	"Quaternion", //DMX_ATTR_QUATERNION = 13,
	"Matrix4x4", //DMX_ATTR_MATRIX4X4  = 14,
	"uint64", //DMX_ATTR_UINT64     = 15,
	"byte", //DMX_ATTR_BYTE       = 16,
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

static_function str ReadBufferGetStr(ReadBuffer *buffer)
{
	str result = {};
	if (buffer && buffer->data)
	{
		if (buffer->readBytes < buffer->size)
		{
			i64 start = buffer->readBytes;
			result.data = (char *)(buffer->data + buffer->readBytes);
			while (*(buffer->data + buffer->readBytes))
			{
				buffer->readBytes++;
				if (buffer->readBytes == buffer->size)
				{
					result = (str){};
					break;
				}
			}
			if (result.data)
			{
				// NOTE(GameChaos): null terminator
				buffer->readBytes++;
				result.length = buffer->readBytes - start;
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
				if (ReadBufferReadI32(buffer, &index) && index >= 0 && index < stringTable->count)
				{
					*(const char **)out = stringTable->strings[index].data;
				}
				ASSERT(index >= 0 && index < stringTable->count);
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
		if (ReadBufferReadI32(buffer, &index) && index >= 0 && index < stringTable->count)
		{
			str string = stringTable->strings[index];
			ASSERT(string.length < sizeof(attr->name));
			Format(attr->name, sizeof(attr->name), "%.*s", (i32)string.length, string.data);
		}
		ASSERT(index >= 0 && index < stringTable->count);
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
	// TODO: print nested elements
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

static_function Dmx DmxImportBinary(const char *path, Arena *arena)
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
		
		// prefix elements and attributes
		i32 prefixElementCount = 0;
		ASSERT(prefixElementCount < I16_MAX);
		ReadBufferReadI32(&buf, &prefixElementCount);
		for (i32 prefixElem = 0; prefixElem < prefixElementCount; prefixElem++)
		{
			DmxElement *elem = DmxAddElement(&result.prefix, NULL, STR(""), STR(""), arena);
			i32 attributeCount = 0;
			ReadBufferReadI32(&buf, &attributeCount);
			ASSERT(attributeCount < I16_MAX);
			for (i32 attrIndex = 0; attrIndex < attributeCount; attrIndex++)
			{
				DmxAttribute *attr = DmxAddAttribute(elem, STR(""), DMX_ATTR_UNKNOWN);
				ReadBufferReadDmxAttribute(&buf, attr, arena, NULL);
			}
		}
		
		// string table
		DmxStringTable stringtable = {};
		ReadBufferReadI32(&buf, &stringtable.count);
		// TODO: put some reasonable cap on stringtable count
		ASSERT(stringtable.count < I16_MAX);
		stringtable.strings = ArenaAlloc(arena, sizeof(*stringtable.strings) * stringtable.count);
		for (i32 stringIndex = 0; stringIndex < stringtable.count; stringIndex++)
		{
			stringtable.strings[stringIndex] = ReadBufferGetStr(&buf);
			MyPrintf("stringtable[$]: $\n", stringIndex, stringtable.strings[stringIndex]);
		}
		
		// element headers
		i32 elementCount = 0;
		ReadBufferReadI32(&buf, &elementCount);
		for (i32 elemIndex = 0; elemIndex < elementCount; elemIndex++)
		{
			//DmxReadElemHeader *elemHeader = &result.elementHeaders[elem];
			i32 typeIndex = 0;
			i32 nameIndex = 0;
			Guid guid = {};
			ReadBufferReadI32(&buf, &typeIndex);
			ReadBufferReadI32(&buf, &nameIndex);
			ReadBufferReadGuid(&buf, &guid);
			// TODO: make this a real check you lazy bum
			ASSERT(typeIndex < stringtable.count && nameIndex < stringtable.count);
			DmxElement *elem = DmxAddElement(&result.body, NULL,
											 stringtable.strings[nameIndex], stringtable.strings[typeIndex], arena);
			// preserve guid
			elem->guid = guid;
		}
		
		// element bodies
		for (i32 elemInd = 0; elemInd < result.body.count; elemInd++)
		{
			DmxElement *elem = &result.body.elements[elemInd];
			i32 attributeCount = 0;
			ReadBufferReadI32(&buf, &attributeCount);
			// TODO: make this a real check you lazy bum
			ASSERT(attributeCount < I16_MAX);
			for (i32 attr = 0; attr < attributeCount; attr++)
			{
				DmxAttribute *attr = DmxAddAttribute(elem, STR(""), DMX_ATTR_UNKNOWN);
				ReadBufferReadDmxAttribute(&buf, attr, arena, &stringtable);
			}
		}
	}
	
	return result;
}

static_function void WriteDmxAttributeValue_(FileWritingBuffer *buffer, void *value, DmxAttrType type, IntStringmap *stringtable)
{
	ASSERT_RANGE(type, 0, DMX_ATTR_COUNT);
	switch (type)
	{
		case DMX_ATTR_UNKNOWN:
		{
			ASSERT(0);
		} break;
		
		case DMX_ATTR_ELEMENT:
		{
			DmxElementId *element = value;
			BufferPushI32(buffer, element->index, false);
			if (element->index == -2)
			{
				BufferPushData(buffer, &element->guid, sizeof(element->guid), false);
			}
		} break;
		
		case DMX_ATTR_INT32:
		case DMX_ATTR_F32:
		case DMX_ATTR_TIMESPAN:
		case DMX_ATTR_RGBA8:
		{
			BufferPushI32(buffer, *(i32 *)value, false);
		} break;
		
		case DMX_ATTR_BOOL:
		case DMX_ATTR_BYTE:
		{
			BufferPushU8(buffer, *(u8 *)value, false);
		} break;
		
		case DMX_ATTR_STRING:
		{
			if (stringtable)
			{
				BufferPushI32(buffer, IntStringmapGet(stringtable, *(const char **)value).value, false);
			}
			else
			{
				BufferWriteCString(buffer, *(const char **)value);
			}
		} break;
		
		case DMX_ATTR_BINARYBLOB:
		{
			DmxBinaryBlob *binaryBlob = value;
			BufferPushI32(buffer, binaryBlob->byteCount, false);
			BufferPushData(buffer, binaryBlob->bytes, binaryBlob->byteCount, false);
		} break;
		
		case DMX_ATTR_VECTOR2D:
		case DMX_ATTR_UINT64:
		{
			BufferPushU64(buffer, *(u64 *)value, false);
		} break;
		
		case DMX_ATTR_VECTOR3D:
		case DMX_ATTR_QANGLE:
		{
			BufferPushV3(buffer, *(v3 *)value, false);
		} break;
		
		case DMX_ATTR_VECTOR4D:
		case DMX_ATTR_QUATERNION:
		{
			BufferPushV4(buffer, *(v4 *)value, false);
		} break;
		
		case DMX_ATTR_MATRIX4X4:
		{
			BufferPushMat4(buffer, *(mat4 *)value, false);
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
		WriteDmxAttributeValue_(buffer, &attr->value.startOfValueData, attr->value.type, stringtable);
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
				if (valueType == DMX_ATTR_STRING)
				{
					valueType = valueType;
				}
				// NOTE(GameChaos): this doesn't reference the stringtable ever
				WriteDmxAttributeValue_(buffer, (u8 *)attr->value.array + containerBytes * i, valueType, NULL);
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
		
		IntStringmapPush(&stringtable, elem->type, (i32)stringtable.length);
		IntStringmapPush(&stringtable, elem->name, (i32)stringtable.length);
		for (i32 attrInd = 0; attrInd < elem->attributeCount; attrInd++)
		{
			DmxAttribute *attr = &elem->attributes[attrInd];
			IntStringmapPush(&stringtable, attr->name, (i32)stringtable.length);
			if (attr->value.type == DMX_ATTR_STRING)
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
		MyPrintf("$: $\n", pair.key, pair.value);
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
			.max = DMX_MAX_PREFIX_ELEMS,
		},
		{
			.count = 0,
			.max = DMX_MAX_ELEMENTS,
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

static_function void DmxAttrSetData(DmxAttribute *attr, const void *data, i64 bytes)
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

static_function DmxAttribute *DmxAddAttributeBinary(DmxElement *parent, str name, const void *binaryBlob, i64 bytes)
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
		result->maxAttributes = DMX_MAX_ATTRIBUTES;
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

static_function void DmxPrint(Dmx dmx)
{
	for (i32 elemsType = 0; elemsType < 2; elemsType++)
	{
		DmxElements *elems = &dmx.body;
		if (elemsType == 0)
		{
			elems = &dmx.prefix;
		}
		for (i32 elemInd = 0; elemInd < elems->count; elemInd++)
		{
			DmxElement *elem = &elems->elements[elemInd];
			MyPrintf("\"$\" \"$\"\n{\n", elem->type, elem->name);
			Print("    \"id\" \"elementid\" \"%08x-%04x-%04x-%04x-%04x%04x%04x\"\n",
				  elem->guid.uints[0], elem->guid.shorts[2], elem->guid.shorts[3],
				  elem->guid.shorts[4], elem->guid.shorts[5], elem->guid.shorts[6],
				  elem->guid.shorts[7]);
			for (i32 attrInd = 0; attrInd < elem->attributeCount; attrInd++)
			{
				DmxAttribute *attr = &elem->attributes[attrInd];
				switch (attr->value.type)
				{
					case DMX_ATTR_INT32:
					case DMX_ATTR_TIMESPAN:
					{
						MyPrintf("    \"$\" \"$\" \"$\"\n",
							  attr->name, g_dmxAttrTypes[attr->value.type % DMX_ATTR_COUNT],
							  attr->value.int32);
					} break;
					
					case DMX_ATTR_F32:
					{
						MyPrintf("    \"$\" \"$\" \"$\"\n",
							  attr->name, g_dmxAttrTypes[attr->value.type % DMX_ATTR_COUNT],
							  attr->value.float32);
					} break;
					
					case DMX_ATTR_BOOL:
					{
						MyPrintf("    \"$\" \"$\" \"$\"\n",
							  attr->name, g_dmxAttrTypes[attr->value.type % DMX_ATTR_COUNT],
							  attr->value.boolean);
					} break;
					
					case DMX_ATTR_STRING:
					{
						MyPrintf("    \"$\" \"$\" \"$\"\n",
							  attr->name, g_dmxAttrTypes[attr->value.type % DMX_ATTR_COUNT],
							  attr->value.string);
					} break;
					
					case DMX_ATTR_RGBA8:
					{
						Print("    \"%s\" \"%s\" \"%08x\"\n",
							  attr->name, g_dmxAttrTypes[attr->value.type % DMX_ATTR_COUNT],
							  attr->value.rgba);
					} break;
					
					case DMX_ATTR_VECTOR2D:
					{
						MyPrintf("    \"$\" \"$\" \"$\"\n",
							  attr->name, g_dmxAttrTypes[attr->value.type % DMX_ATTR_COUNT],
							  attr->value.vector2);
					} break;
					
					case DMX_ATTR_VECTOR3D:
					case DMX_ATTR_QANGLE:
					case DMX_ATTR_QUATERNION:
					{
						MyPrintf("    \"$\" \"$\" \"$\"\n",
							  attr->name, g_dmxAttrTypes[attr->value.type % DMX_ATTR_COUNT],
							  attr->value.vector3);
					} break;
					
					case DMX_ATTR_VECTOR4D:
					{
						MyPrintf("    \"$\" \"$\" \"$\"\n",
							  attr->name, g_dmxAttrTypes[attr->value.type % DMX_ATTR_COUNT],
							  attr->value.vector4);
					} break;
					
					case DMX_ATTR_UINT64:
					{
						MyPrintf("    \"$\" \"$\" \"$\"\n",
							  attr->name, g_dmxAttrTypes[attr->value.type % DMX_ATTR_COUNT],
							  attr->value.uint64);
					} break;
					
					case DMX_ATTR_BYTE:
					{
						MyPrintf("    \"$\" \"$\" \"$\"\n",
							  attr->name, g_dmxAttrTypes[attr->value.type % DMX_ATTR_COUNT],
							  attr->value.byte);
					} break;
					
					default:
					{
						MyPrintf("    \"$\" \"$\" \"\"\n",
							  attr->name, g_dmxAttrTypes[attr->value.type % DMX_ATTR_COUNT]);
					} break;
				}
			}
			Print("%s", "}\n");
		}
	}
}

static_function void DmxTest(Arena *arena, Arena *tempArena)
{
	// load test
	{
		MyPrintf("sizeof(DmxElement) = $, sizeof(DmxAttribute) = $\n", sizeof(DmxElement), sizeof(DmxAttribute));
		MyPrintf("memory allocated per element: $KB\n",
			  ((sizeof(DmxElement) + sizeof(DmxAttribute) * DMX_MAX_ATTRIBUTES)) / KILOBYTES(1));
		MyPrintf("max memory allocated per dmx: $MB\n",
			  ((sizeof(DmxElement) + sizeof(DmxAttribute) * DMX_MAX_ATTRIBUTES) * DMX_MAX_ELEMENTS) / MEGABYTES(1));
		Dmx dmxTest = DmxImportBinary("debug/empty.vmap", arena);
		//Dmx dmxTest = DmxImportBinary("debug/kz_victoria.vmap", arena);
		//DmxReadBinary_v9 dmxTest = DmxImportBinary("debug/empty.vmap", arena);
		//DmxReadBinary_v9 dmxTest = DmxImportBinary("debug/out.vmap", arena);
		
		// reexport test
		DmxExportBinary("debug/out.vmap", arena, dmxTest);
		
		DmxPrint(dmxTest);
		Dmx dmxTest2 = DmxImportBinary("debug/out.vmap", arena);
		DmxPrint(dmxTest2);
		
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