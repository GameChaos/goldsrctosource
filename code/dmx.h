/* date = September 28th 2023 1:44 pm */

#ifndef DMX_H
#define DMX_H

// TODO: support other dmx types
#define DMX_V9_BIN_HEADER "<!-- dmx encoding binary 9 format %s %i -->\n"
#define DMX_V9_BIN_HEADER_START "<!-- dmx encoding binary 9 format %s %i -->\n"

#define DMX_MAX_NAME_LEN 64
#define DMX_MAX_PREFIX_ELEMS 64
#define DMX_MAX_ATTRIBUTES 1024
#define DMX_MAX_ELEMENTS 8192
#define DMX_ENCODING_VERSION 9

typedef enum DmxAttrType_s : u8
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
} DmxAttrType;
static_assert(sizeof(DMX_ATTR_COUNT) == 1);

typedef enum
{
	DMXENCODING_INVALID,
	DMXENCODING_BINARY, // currently only support binary.
	//DMXENCODING_KEYVALUES2,
} DmxEncoding;

typedef union
{
	u8 bytes[16];
	u16 shorts[8];
	u32 uints[4];
} Guid;

typedef struct
{
	DmxEncoding encoding;
	i32 encodingVersion;
	char format[64];
	i32 formatVersion;
} DmxHeader;

typedef struct
{
	i32 index;
	Guid guid;
} DmxElementId;

typedef struct
{
	i32 byteCount;
	u8 *bytes;
} DmxBinaryBlob;

typedef struct
{
	DmxAttrType type;
	union
	{
		u8 startOfValueData;
		
		DmxElementId element;
		i32 int32;
		f32 float32;
		bool boolean;
		char *string; // DMX_ATTR_STRING only in the prefix
		i32 stringIndex; // DMX_ATTR_STRING
		DmxBinaryBlob binaryBlob;
		i32 timespan;
		i32 rgba;
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
			i32 arrayCount;
			union
			{
				void *array; // for custom types
				
				DmxElementId *elementArray;
				i32 *int32Array;
				f32 *float32Array;
				bool *booleanArray;
				char **stringArray; // DMX_ATTR_STRING
				DmxBinaryBlob *binaryBlobArray;
				i32 *timespanArray;
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
} DmxAttrValue;

typedef struct
{
	char name[DMX_MAX_NAME_LEN];
	DmxAttrValue value;
} DmxAttribute;

typedef struct
{
	i32 count;
    str *strings;
} DmxStringTable;

typedef struct
{
	char type[DMX_MAX_NAME_LEN];
	char name[DMX_MAX_NAME_LEN];
	Guid guid;
	i32 maxAttributes;
	i32 attributeCount;
	DmxAttribute *attributes;
} DmxElement;
// NOTE(GameChaos): roughly curb the maximum size of a dmx
static_assert(((sizeof(DmxElement) + sizeof(DmxAttribute) * DMX_MAX_ATTRIBUTES) * DMX_MAX_ELEMENTS) / MEGABYTES(1) < MEGABYTES(1536));

typedef struct
{
	i32 count;
	i32 max;
	DmxElement *elements;
} DmxElements;

typedef struct
{
	// NOTE: only 1 prefix element for now.
	DmxHeader header;
	DmxElements prefix;
	DmxElements body;
} Dmx;




typedef struct
{
	char *type;
	char *name;
	Guid guid;
} DmxReadElemHeader;

typedef struct
{
	i32 attributeCount;
	DmxAttribute *attributes;
} DmxReadElemBody;

typedef struct
{
	ReadFileResult file;
	char *header; // "<!-- dmx encoding binary 9 format %s %i -->\n"
	i32 prefixElementCount;
	DmxReadElemBody *prefixElements;
	DmxStringTable stringTable;
	i32 elementCount;
	DmxReadElemHeader *elementHeaders;
	DmxReadElemBody *elements;
} DmxReadBinary_v9;

#define DEFINE_DMXADDATTRIBUTE_FUNC_SIG(functionName, dataType)\
static_function DmxAttribute *functionName(DmxElement *parent, str name, dataType value)
#define DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(functionName, dataType)\
static_function DmxAttribute *functionName(DmxElement *parent, str name, dataType *items, i32 itemCount)

static_function Dmx DmxCreate(Arena *arena, const char *format, i32 formatVersion);
static_function DmxAttribute *DmxAddAttribute(DmxElement *parent, str name, DmxAttrType type);
static_function void DmxAttrSetData(DmxAttribute *attr, void *data, i64 bytes);
static_function DmxElement *DmxAddElement(DmxElements *elements, DmxElement *parent, str name, str type, Arena *arena);

DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeElementId, DmxElementId);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeInt, i32);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeF32, f32);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeBool, bool);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeTimespan, i32);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeRgba8, u32);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeV2, v2);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeV3, v3);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeV4, v4);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeQAngle, v3);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeQuaternion, v4);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeMat4, mat4);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeU64, u64);
DEFINE_DMXADDATTRIBUTE_FUNC_SIG(DmxAddAttributeU8, u8);

static_function DmxAttribute *DmxAddAttributeBinary(DmxElement *parent, str name, void *binaryBlob, i64 bytes);
static_function DmxAttribute *DmxAddAttributeString(DmxElement *parent, str name, str value);

DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayElementId, DmxElementId);
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayInt, i32);
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayBool, bool);
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayString, char *);
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayBinary, DmxBinaryBlob);
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayTimespan, i32);
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayRgba8, u32);
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayV2, v2);
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayV3, v3);
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayV4, v4);
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayQAngle, v3);
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayQuaternion, v4);
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayMat4, mat4);
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayU64, u64);
DEFINE_DMXADDATTRIBUTEARRAY_FUNC_SIG(DmxAddAttributeArrayU8, u8);

#endif //DMX_H
