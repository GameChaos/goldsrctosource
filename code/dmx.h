/* date = September 28th 2023 1:44 pm */

#ifndef DMX_H
#define DMX_H

#define DMX_V9_BIN_HEADER "<!-- dmx encoding binary 9 format %s %i -->\n"

#define DMX_MAX_NAME_LEN 64
#define DMX_MAX_PREFIX_ELEMS 64
#define DMX_DEFAULT_MAX_ATTRIBUTES 64

enum DmxAttrType : u8
{
	DMX_ATTR_COUNT = 16,
	DMX_ATTR_UNKNOWN = 0,
	
	DMX_ATTR_ELEMENT    = 1,
	DMX_ATTR_INT32      = 2,
	DMX_ATTR_F32        = 3,
	DMX_ATTR_BOOL       = 4,
	DMX_ATTR_STRING     = 5,
	DMX_ATTR_BINARYBLOB = 6,
	DMX_ATTR_TIMESPAN   = 7,
	DMX_ATTR_RGBA8      = 8,
	DMX_ATTR_VECTOR2D   = 9,
	DMX_ATTR_VECTOR3D   = 10,
	DMX_ATTR_VECTOR4D   = 11,
	DMX_ATTR_QANGLE     = 12,
	DMX_ATTR_QUATERNION = 13,
	DMX_ATTR_MATRIX4X4  = 14,
	DMX_ATTR_UINT64     = 15,
	DMX_ATTR_BYTE       = 16,
	
	// NOTE(GameChaos): these are seemingly never used in CS2 .vmaps
#if 0
	DMX_ATTR_ELEMENT_ARRAY    = 17,
	DMX_ATTR_INT32_ARRAY      = 18,
	DMX_ATTR_F32_ARRAY        = 19,
	DMX_ATTR_BOOL_ARRAY       = 20,
	DMX_ATTR_STRING_ARRAY     = 21,
	DMX_ATTR_BINARYBLOB_ARRAY = 22,
	DMX_ATTR_TIMESPAN_ARRAY   = 23,
	DMX_ATTR_RGBA8_ARRAY      = 24,
	DMX_ATTR_VECTOR2D_ARRAY   = 25,
	DMX_ATTR_VECTOR3D_ARRAY   = 26,
	DMX_ATTR_VECTOR4D_ARRAY   = 27,
	DMX_ATTR_QANGLE_ARRAY     = 28,
	DMX_ATTR_QUATERNION_ARRAY = 29,
	DMX_ATTR_MATRIX4X4_ARRAY  = 30,
	DMX_ATTR_UINT64_ARRAY     = 31,
	DMX_ATTR_BYTE_ARRAY       = 32,
#endif
	
	// NOTE(GameChaos): these are used instead of the upper ones in CS2 vmaps for some reason
	DMX_ATTR_ELEMENT_ARRAY    = 33,
	DMX_ATTR_INT32_ARRAY      = 34,
	DMX_ATTR_F32_ARRAY        = 35,
	DMX_ATTR_BOOL_ARRAY       = 36,
	DMX_ATTR_STRING_ARRAY     = 37,
	DMX_ATTR_BINARYBLOB_ARRAY = 38,
	DMX_ATTR_TIMESPAN_ARRAY   = 39,
	DMX_ATTR_RGBA8_ARRAY      = 40,
	DMX_ATTR_VECTOR2D_ARRAY   = 41,
	DMX_ATTR_VECTOR3D_ARRAY   = 42,
	DMX_ATTR_VECTOR4D_ARRAY   = 43,
	DMX_ATTR_QANGLE_ARRAY     = 44,
	DMX_ATTR_QUATERNION_ARRAY = 45,
	DMX_ATTR_MATRIX4X4_ARRAY  = 46,
	DMX_ATTR_UINT64_ARRAY     = 47,
	DMX_ATTR_BYTE_ARRAY       = 48,
};

union Guid
{
	u8 bytes[16];
	u32 uints[4];
};

struct DmxElementId
{
	s32 index;
	Guid guid;
};

struct DmxBinaryBlob
{
	s32 byteCount;
	u8 *bytes;
};

struct DmxAttrValue
{
	DmxAttrType type;
	union
	{
		u8 startOfValueData;
		
		DmxElementId element;
		s32 int32;
		f32 float32;
		bool boolean;
		char *string; // DMX_ATTR_STRING only in the prefix
		s32 stringIndex; // DMX_ATTR_STRING
		DmxBinaryBlob binaryBlob;
		s32 timespan;
		s32 rgba;
		v2 vector2;
		v3 vector3;
		v4 vector4;
		v3 qangle;
		v4 quaternion;
		mat4 matrix4x4;
		u64 uint64;
		u8 byte;
		
		// array types
		struct
		{
			s32 arrayCount;
			union
			{
				void *array;
				
				DmxElementId *elementArray;
				s32 *int32Array;
				f32 *float32Array;
				bool *booleanArray;
				char **stringArray; // DMX_ATTR_STRING
				DmxBinaryBlob *binaryBlobArray;
				s32 *timespanArray;
				u32 *rgbaArray;
				v2 *vector2Array;
				v3 *vector3Array;
				v4 *vector4Array;
				v3 *qangleArray;
				v4 *quaternionArray;
				mat4 *matrix4x4Array;
				u64 *uint64Array;
				u8 *byteArray;
			};
		};
	};
};

struct DmxAttribute
{
	char name[DMX_MAX_NAME_LEN];
	DmxAttrValue value;
};

struct DmxStringTable
{
	s32 stringCount;
    char **strings;
};

struct DmxElement
{
	char type[DMX_MAX_NAME_LEN];
	char name[DMX_MAX_NAME_LEN];
	Guid guid;
	s32 maxAttributes;
	s32 attributeCount;
	DmxAttribute *attributes;
};

struct Dmx
{
	// NOTE: only 1 prefix element for now.
	DmxElement prefix;
	
	//s32 maxStrings;
	//s64 maxStringBytes;
	//s64 currentStringByte;
	//char *stringMemory;
	//DmxStringTable stringTable;
	
	s32 elementCount;
	s32 maxElements;
	DmxElement *elements;
};




struct DmxReadElemHeader
{
	char *type;
	char *name;
	Guid guid;
};

struct DmxReadElemBody
{
	s32 attributeCount;
	DmxAttribute *attributes;
};

struct DmxReadBinary_v9
{
	ReadFileResult file;
	char *header; // "<!-- dmx encoding binary 9 format %s %i -->\n"
	s32 prefixElementCount;
	DmxReadElemBody *prefixElements;
	DmxStringTable stringTable;
	s32 elementCount;
	DmxReadElemHeader *elementHeaders;
	DmxReadElemBody *elements;
};

#define DEFINE_DMXADDATTRIBUTE_FUNC_SIG(functionName, dataType)\
internal DmxAttribute *functionName(Dmx *dmx, DmxElement *parent, str name, dataType value)

internal Dmx DmxCreate(Arena *arena);
internal DmxElement *DmxGetPrefix(Dmx *dmx);

internal DmxElement *DmxAddElement(Dmx *dmx, DmxElement *parent, str name, str type, Arena *arena);

internal DmxAttribute *DmxAddAttribute(Dmx *dmx, DmxElement *parent, str name, DmxAttrType type);

internal void DmxAttrSetData(Dmx *dmx, DmxAttribute *attr, void *data, s64 bytes);

DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeElementId, DmxElementId);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeInt, s32);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeBool, bool);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeTimespan, s32);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeRgba8, u32);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeV2, v2);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeV3, v3);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeV4, v4);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeQAngle, v3);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeQuaternion, v4);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeMat4, mat4);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeU64, u64);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeU8, u8);

internal DmxAttribute *DmxAddAttributeBinary(Dmx *dmx, DmxElement *parent, str name, void *binaryBlob, s64 bytes);
internal DmxAttribute *DmxAddAttributeString(Dmx *dmx, DmxElement *parent, str name, str value);

#endif //DMX_H
