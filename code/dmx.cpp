
#include "dmx.h"

// NOTE(GameChaos): Parts taken from: https://github.com/kristiker/Datamodel.NET

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
			Mem_Copy(buffer->data + buffer->readBytes, out, outSize);
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
DEFINE_READINGBUFFER_READ_TYPE(ReadBufferReadGuid, Guid);

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
	sizeof(DmxElementId), // DMX_ATTR_ELEMENT
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
		case DMX_ATTR_UNKNOWN:
		{
			ASSERT(0);
		} break;
		case DMX_ATTR_ELEMENT:
		{
			DmxElementId *element = (DmxElementId *)out;
			ReadBufferReadS32(buffer, &element->index);
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

internal DmxReadBinary_v9 DmxImportBinary(const char *path, Arena *arena)
{
	DmxReadBinary_v9 result = {};
	result.file = ReadEntireFile(arena, path);
	
	if (result.file.contents)
	{
		ReadBuffer buf = ReadBufferCreate(result.file.contents, result.file.size);
		result.header = ReadBufferGetString(&buf);
		
		// prefix elements and attributes
		ReadBufferReadS32(&buf, &result.prefixElementCount);
		result.prefixElements = (DmxReadElemBody *)ArenaAlloc(arena, sizeof(*result.prefixElements) * result.prefixElementCount);
		for (s32 prefixElem = 0; prefixElem < result.prefixElementCount; prefixElem++)
		{
			DmxReadElemBody *element = &result.prefixElements[prefixElem];
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
		result.elementHeaders = (DmxReadElemHeader *)ArenaAlloc(arena, sizeof(*result.elementHeaders) * result.elementCount);
		for (s32 elem = 0; elem < result.elementCount; elem++)
		{
			DmxReadElemHeader *elemHeader = &result.elementHeaders[elem];
			s32 typeIndex = 0;
			s32 nameIndex = 0;
			ReadBufferReadS32(&buf, &typeIndex);
			ReadBufferReadS32(&buf, &nameIndex);
			ReadBufferReadGuid(&buf, &elemHeader->guid);
			elemHeader->type = result.stringTable.strings[typeIndex];
			elemHeader->name = result.stringTable.strings[nameIndex];
		}
		
		// element bodies
		result.elements = (DmxReadElemBody *)ArenaAlloc(arena, sizeof(*result.elements) * result.elementCount);
		for (s32 elem = 0; elem < result.elementCount; elem++)
		{
			DmxReadElemBody *element = &result.elements[elem];
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

#if 0
internal void DmxExportBinary(char *path, Arena *arena, DmxReadBinary_v9 dmx)
{
	FileWritingBuffer buf = BufferCreate(arena, GIGABYTES(1));
	
	BufferPushData(&buf, DMX_V9_BIN_HEADER, sizeof(DMX_V9_BIN_HEADER), false);
	BufferPushData(&buf, &dmx.prefixElementCount, sizeof(dmx.prefixElementCount), false);
	for (s32 elemInd = 0; elemInd < dmx.prefixElementCount; elemInd++)
	{
		DmxReadElemBody *elem = &dmx.prefixElements[elemInd];
		BufferPushData(&buf, &elem->attributeCount, sizeof(elem->attributeCount), false);
		
		for (s32 attrInd = 0; attrInd < elem->attributeCount; attrInd++)
		{
			DmxAttribute *attr = &elem->attributes[attrInd];
			BufferPushData(&buf, &attr->name, strlen(attr->name) + 1, false);
			BufferPushData(&buf, &attr->value.type, sizeof(attr->value.type), false);
			
		}
	}
}
#endif

internal Dmx DmxCreate(Arena *arena)
{
	Dmx result = {};
	
	result.prefix.attributes = (DmxAttribute *)ArenaAlloc(arena, DMX_DEFAULT_MAX_ATTRIBUTES * sizeof(*result.prefix.attributes));
	//result.maxStrings = 1024;
	//result.maxStringBytes = 1024 * 64;
	//result.stringTable.strings = (char **)ArenaAlloc(arena, result.maxStrings * sizeof(*result.stringTable.strings));
	//result.stringMemory = (char *)ArenaAlloc(arena, result.maxStringBytes);
	
	result.maxElements = 1024;
	result.elements = (DmxElement *)ArenaAlloc(arena, result.maxElements * sizeof(*result.elements));
	
	result.prefix = *DmxAddElement(&result, NULL, STR(""), STR(""), arena);
	result.elementCount = 0;
	
	return result;
}

internal DmxElement *DmxGetPrefix(Dmx *dmx)
{
	DmxElement *result = &dmx->prefix;
	return result;
}

// NOTE(GameChaos): i hope source 2 doesn't care that my guids don't conform to the standard.
internal Guid GenerateGuid()
{
	Guid result = {};
	for (s32 i = 0; i < 4; i++)
	{
		result.uints[i] = pcg32_random();
	}
	return result;
}

internal DmxAttribute *DmxAddAttribute(Dmx *dmx, DmxElement *parent, str name, DmxAttrType type)
{
	ASSERT(parent && name.data);
	DmxAttribute *result = NULL;
	if (parent->attributeCount < parent->maxAttributes)
	{
		result = &parent->attributes[parent->attributeCount++];
		*result = {};
		result->value.type = type;
		Format(result->name, sizeof(result->name), "%.*s", (s32)name.length, name.data);
	}
	ASSERT(result);
	
	return result;
}

internal void DmxAttrSetData(Dmx *dmx, DmxAttribute *attr, void *data, s64 bytes)
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
    DmxAttribute *result = DmxAddAttribute(dmx, parent, name, attrType);\
    if (result)\
    {\
        DmxAttrSetData(dmx, result, &(value), (s64)sizeof(value));\
    }\
    return result;\
}\

DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeElementId, DmxElementId, DMX_ATTR_ELEMENT)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeInt, s32, DMX_ATTR_INT32)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeF32, s32, DMX_ATTR_F32)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeBool, bool, DMX_ATTR_BOOL)
DEFINE_DMXADDATTRIBUTE_FUNC(DmxAddAttributeTimespan, s32, DMX_ATTR_TIMESPAN)
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

internal DmxAttribute *DmxAddAttributeBinary(Dmx *dmx, DmxElement *parent, str name, void *binaryBlob, s64 bytes)
{
	DmxAttribute *result = DmxAddAttribute(dmx, parent, name, DMX_ATTR_BINARYBLOB);
    if (result)
    {
        DmxAttrSetData(dmx, result, binaryBlob, bytes);
    }
    return result;
}

internal DmxAttribute *DmxAddAttributeString(Dmx *dmx, DmxElement *parent, str name, str value)
{
	DmxAttribute *result = DmxAddAttribute(dmx, parent, name, DMX_ATTR_STRING);
    if (result)
    {
        DmxAttrSetData(dmx, result, (void *)value.data, 1);
    }
    return result;
}

#define DEFINE_DMXADDATTRIBUTEARRAY_FUNC(functionName, dataType, attrType)\
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(functionName, dataType)\
{\
    DmxAttribute *result = DmxAddAttribute(dmx, parent, name, attrType);\
    if (result)\
    {\
        result->value.arrayCount = itemCount;\
        result->value.array = items;\
    }\
    return result;\
}\

DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayElementId, DmxElementId, DMX_ATTR_ELEMENT_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayInt, s32, DMX_ATTR_INT32_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayBool, bool, DMX_ATTR_BOOL_ARRAY)
DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxAddAttributeArrayTimespan, s32, DMX_ATTR_TIMESPAN_ARRAY)
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

internal DmxElement *DmxCreateElement(Dmx *dmx, str name, str type, Arena *arena)
{
	ASSERT(name.data && type.data);
	DmxElement *result = NULL;
	if (dmx->elementCount < dmx->maxElements)
	{
		result = &dmx->elements[dmx->elementCount++];
		*result = {};
		result->maxAttributes = 1024;
		result->attributes = (DmxAttribute *)ArenaAlloc(arena, result->maxAttributes * sizeof(*result->attributes));
		Format(result->type, sizeof(result->type), "%.*s", (s32)type.length, type.data);
		Format(result->name, sizeof(result->name), "%.*s", (s32)name.length, name.data);
		result->guid = GenerateGuid();
	}
	ASSERT(result);
	return result;
}

internal DmxElement *DmxAddElement(Dmx *dmx, DmxElement *parent, str name, str type, Arena *arena)
{
	DmxElement *result = DmxCreateElement(dmx, name, type, arena);
	if (result)
	{
		DmxElementId id = {
			dmx->elementCount - 1,
			result->guid
		};
		if (parent)
		{
			DmxAttribute *attr = DmxAddAttributeElementId(dmx, parent, name, id);
			ASSERT(attr);
		}
	}
	ASSERT(result);
	return result;
}

internal void DmxTest(Arena *arena, Arena *tempArena)
{
	// load test
	{
		DmxReadBinary_v9 dmxTest = DmxImportBinary("debug/empty.vmap", arena);
		
#if 1
		for (s32 elem = 0; elem < dmxTest.prefixElementCount; elem++)
		{
			DmxReadElemBody *elemBody = &dmxTest.prefixElements[elem];
			Print("{\n");
			for (s32 attrInd = 0; attrInd < elemBody->attributeCount; attrInd++)
			{
				DmxAttribute *attr = &elemBody->attributes[attrInd];
				Print("\t\"%s\" \"%i\" \"\"\n", attr->name, attr->value.type);
			}
			Print("%s", "}\n");
		}
		
		for (s32 elem = 0; elem < dmxTest.elementCount; elem++)
		{
			DmxReadElemHeader *elemHeader = &dmxTest.elementHeaders[elem];
			DmxReadElemBody *elemBody = &dmxTest.elements[elem];
			Print("\"%s\" \"%s\"\n{\n", elemHeader->type, elemHeader->name);
			for (s32 attrInd = 0; attrInd < elemBody->attributeCount; attrInd++)
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
		Dmx dmx = DmxCreate(arena);
		
		// prefix attributes
		{
			const char *imgData = "P3\n2 2\n0 0 0\n0 0 0\n0 0 0\n0 0 0";
			s64 imgBytes = strlen(imgData);
			DmxAddAttributeBinary(&dmx, DmxGetPrefix(&dmx), STR("asset_preview_thumbnail"), (void *)imgData, imgBytes);
			DmxAddAttributeString(&dmx, DmxGetPrefix(&dmx), STR("asset_preview_thumbnail_format"), STR("ppm"));
			DmxAddAttribute(&dmx, DmxGetPrefix(&dmx), STR("map_asset_references"), DMX_ATTR_STRING_ARRAY);
		}
		
		// CMapRootElement
		{
			DmxElement *mapRootElement = DmxAddElement(&dmx, NULL, STR(""), STR("CMapRootElement"), arena);
			DmxAddAttributeBool(&dmx, mapRootElement, STR("isprefab"), false);
			DmxAddAttributeInt(&dmx, mapRootElement, STR("editorbuild"), 9820);
			DmxAddAttributeInt(&dmx, mapRootElement, STR("editorversion"), 400);
			DmxAddAttributeString(&dmx, mapRootElement, STR("itemFile"), STR(""));
			
			// defaultcamera
			{
				DmxElement *defaultCamera = DmxAddElement(&dmx, mapRootElement, STR("defaultcamera"), STR("CStoredCamera"), arena);
				DmxAddAttributeInt(&dmx, defaultCamera, STR("activecamera"), -1);
				DmxAddAttributeV3(&dmx, defaultCamera, STR("position"), Vec3(0, -1000, -1000));
				DmxAddAttributeV3(&dmx, defaultCamera, STR("lookat"), Vec3(-0.0000000618f, -999.2929077148f, 999.2929077148f));
			}
			
			// 3dcameras
			{
				DmxElement *cameras = DmxAddElement(&dmx, mapRootElement, STR("3dcameras"), STR("CStoredCameras"), arena);
				DmxAddAttributeArrayElementId(&dmx, cameras, STR("cameras"), NULL, 0);
			}
			
			// world
			{
				DmxElement *world = DmxAddElement(&dmx, mapRootElement, STR("world"), STR("CMapWorld"), arena);
				// TODO: correct nodeid
				DmxAddAttributeInt(&dmx, mapRootElement, STR("nodeID"), 1);
				DmxAddAttributeU64(&dmx, mapRootElement, STR("referenceID"), 0);
				DmxAddAttributeArrayElementId(&dmx, mapRootElement, STR("children"), NULL, 0);
				//DmxAddAttributeArrayString(&dmx, mapRootElement, STR("variableTargetKeys"), NULL, 0);
				//DmxAddAttributeArrayString(&dmx, mapRootElement, STR("variableNames"), NULL, 0);
			}
		}
	}
#endif
	return;
}