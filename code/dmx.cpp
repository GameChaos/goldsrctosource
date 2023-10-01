
#include "dmx.h"

struct ReadBuffer
{
	u8 *data;
	s64 size;
	s64 readBytes;
};

internal ReadBuffer ReadBufferCreate(void *data, s64 size)
{
	ReadBuffer result = {};
	result.data = (u8 *)data;
	result.size = size;
	return result;
}

internal b32 ReadBufferRead(ReadBuffer *buffer, void *out, s64 outSize)
{
	b32 result = false;
	if (buffer && buffer->data)
	{
		if (buffer->readBytes + outSize <= buffer->size)
		{
			Mem_Copy(buffer->data + buffer->readBytes, out, outSize, outSize);
			buffer->readBytes += outSize;
			result = true;
		}
	}
	return result;
}

internal u8 *ReadBufferGetBytes(ReadBuffer *buffer, s64 bytes)
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
internal b32 functionName(ReadBuffer *buffer, type *out)\
{\
b32 result = ReadBufferRead(buffer, out, sizeof(*out));\
return result;\
}\

DEFINE_READINGBUFFER_READ_TYPE(ReadBufferReadU8, u8);
DEFINE_READINGBUFFER_READ_TYPE(ReadBufferReadS32, s32);
DEFINE_READINGBUFFER_READ_TYPE(ReadBufferReadU64, u64);
DEFINE_READINGBUFFER_READ_TYPE(ReadBufferReadV3, v3);
DEFINE_READINGBUFFER_READ_TYPE(ReadBufferReadV4, v4);
DEFINE_READINGBUFFER_READ_TYPE(ReadBufferReadMat4, mat4);

internal char *ReadBufferGetString(ReadBuffer *buffer)
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

global s32 g_attrValueContainerSize[] = {
	0, // DMX_ATTR_UNKNOWN
	sizeof(DmxElement), // DMX_ATTR_ELEMENT
	sizeof(s32), // DMX_ATTR_INT32
	sizeof(f32), // DMX_ATTR_F32
	sizeof(bool), // DMX_ATTR_BOOL
	sizeof(char *), // DMX_ATTR_STRING
	sizeof(DmxBinaryBlob), // DMX_ATTR_BINARYBLOB
	sizeof(s32), // DMX_ATTR_TIMESPAN
	sizeof(s32), // DMX_ATTR_RGBA8
	sizeof(v2), // DMX_ATTR_VECTOR2D
	sizeof(v3), // DMX_ATTR_VECTOR3D
	sizeof(v4), // DMX_ATTR_VECTOR4D
	sizeof(v3), // DMX_ATTR_QANGLE
	sizeof(v4), // DMX_ATTR_QUATERNION
	sizeof(mat4), // DMX_ATTR_MATRIX4X4
	sizeof(u64), // DMX_ATTR_UINT64
	sizeof(u8), // DMX_ATTR_BYTE
};

internal void ReadBufferReadDmxAttributeValue_(ReadBuffer *buffer, void *out, DmxAttrType type, DmxStringTable *stringTable = NULL)
{
	switch (type)
	{
		case DMX_ATTR_UNKNOWN: break;
		case DMX_ATTR_ELEMENT:
		{
			DmxElement *element = (DmxElement *)out;
			ReadBufferReadS32(buffer, &element->index);
			if (element->index == -2)
			{
				element->guid = ReadBufferGetString(buffer);
			}
		} break;
		
		case DMX_ATTR_INT32:
		case DMX_ATTR_F32:
		case DMX_ATTR_TIMESPAN:
		case DMX_ATTR_RGBA8:
		{
			ReadBufferReadS32(buffer, (s32 *)out);
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
				s32 index = 0;
				if (ReadBufferReadS32(buffer, &index) && index >= 0 && index < stringTable->stringCount)
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
			ReadBufferReadS32(buffer, &blob->byteCount);
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

internal b32 ReadBufferReadDmxAttribute(ReadBuffer *buffer, DmxAttribute *attr, Arena *arena, DmxStringTable *stringTable = NULL)
{
	b32 result = false;
	if (stringTable)
	{
		s32 index = 0;
		if (ReadBufferReadS32(buffer, &index) && index >= 0 && index < stringTable->stringCount)
		{
			attr->name = stringTable->strings[index];
		}
		ASSERT(index >= 0 && index < stringTable->stringCount);
	}
	else
	{
		attr->name = ReadBufferGetString(buffer);
	}
	ReadBufferReadU8(buffer, (u8 *)&attr->value.type);
	if (attr->value.type <= DMX_ATTR_COUNT)
	{
		ReadBufferReadDmxAttributeValue_(buffer, &attr->value.startOfValueData, attr->value.type, stringTable);
	}
	else
	{
		DmxAttrType valueType = (DmxAttrType)(attr->value.type - DMX_ATTR_COUNT);
		if (attr->value.type > DMX_ATTR_COUNT)
		{
			valueType = (DmxAttrType)(valueType - DMX_ATTR_COUNT);
		}
		ASSERT(valueType <= DMX_ATTR_COUNT);
		ReadBufferReadS32(buffer, &attr->value.arrayCount);
		if (valueType <= DMX_ATTR_COUNT)
		{
			s32 containerBytes = g_attrValueContainerSize[valueType];
			attr->value.array = ArenaAlloc(arena, attr->value.arrayCount * containerBytes);
			void *array = attr->value.array;
			for (s32 i = 0; i < attr->value.arrayCount; i++)
			{
				// NOTE(GameChaos): this doesn't reference the stringtable ever
				ReadBufferReadDmxAttributeValue_(buffer, (u8 *)array + containerBytes * i, valueType);
			}
		}
	}
	
	return result;
}

internal DmxBinary_v9 DmxImportBinary(char *path, Arena *arena)
{
	DmxBinary_v9 result = {};
	result.file = ReadEntireFile(arena, path);
	
	if (result.file.contents)
	{
		ReadBuffer buf = ReadBufferCreate(result.file.contents, result.file.size);
		result.header = ReadBufferGetString(&buf);
		
		// prefix elements and attributes
		ReadBufferReadS32(&buf, &result.prefixElementCount);
		result.prefixElements = (DmxPrefixElement *)ArenaAlloc(arena, sizeof(*result.prefixElements) * result.prefixElementCount);
		for (s32 prefixElem = 0; prefixElem < result.prefixElementCount; prefixElem++)
		{
			DmxPrefixElement *element = &result.prefixElements[prefixElem];
			ReadBufferReadS32(&buf, &element->attributeCount);
			element->attributes = (DmxAttribute *)ArenaAlloc(arena, sizeof(*element->attributes) * element->attributeCount);
			for (s32 attr = 0; attr < element->attributeCount; attr++)
			{
				ReadBufferReadDmxAttribute(&buf, &element->attributes[attr], arena);
			}
		}
		
		// string table
		ReadBufferReadS32(&buf, &result.stringTable.stringCount);
		result.stringTable.strings = (char **)ArenaAlloc(arena, sizeof(*result.stringTable.strings) * result.stringTable.stringCount);
		for (s32 stringIndex = 0; stringIndex < result.stringTable.stringCount; stringIndex++)
		{
			result.stringTable.strings[stringIndex] = ReadBufferGetString(&buf);
		}
		
		// element headers
		ReadBufferReadS32(&buf, &result.elementCount);
		result.elementHeaders = (DmxElementHeader *)ArenaAlloc(arena, sizeof(*result.elementHeaders) * result.elementCount);
		for (s32 elem = 0; elem < result.elementCount; elem++)
		{
			DmxElementHeader *elemHeader = &result.elementHeaders[elem];
			s32 typeIndex = 0;
			s32 nameIndex = 0;
			ReadBufferReadS32(&buf, &typeIndex);
			ReadBufferReadS32(&buf, &nameIndex);
			ReadBufferRead(&buf, elemHeader->guid, sizeof(elemHeader->guid));
			elemHeader->type = result.stringTable.strings[typeIndex];
			elemHeader->name = result.stringTable.strings[nameIndex];
		}
		
		// element bodies
		result.elements = (DmxElementBody *)ArenaAlloc(arena, sizeof(*result.elements) * result.elementCount);
		for (s32 elem = 0; elem < result.elementCount; elem++)
		{
			DmxElementBody *element = &result.elements[elem];
			ReadBufferReadS32(&buf, &element->attributeCount);
			element->attributes = (DmxAttribute *)ArenaAlloc(arena, sizeof(*element->attributes) * element->attributeCount);
			for (s32 attr = 0; attr < element->attributeCount; attr++)
			{
				ReadBufferReadDmxAttribute(&buf, &element->attributes[attr], arena, &result.stringTable);
			}
		}
	}
	
	return result;
}

internal void DmxTest(Arena *arena)
{
	DmxBinary_v9 dmxTest = DmxImportBinary("debug/square.vmap", arena);
	
	for (s32 elem = 0; elem < dmxTest.elementCount; elem++)
	{
		DmxElementHeader *elemHeader = &dmxTest.elementHeaders[elem];
		DmxElementBody *elemBody = &dmxTest.elements[elem];
		Print("%s %s;\n", elemHeader->type, elemHeader->name);
	}
	
	return;
}