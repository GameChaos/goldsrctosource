/* date = September 28th 2023 1:44 pm */

#ifndef DMX_H
#define DMX_H

enum DmxAttrType : u8
{
	DMX_ATTR_COUNT = 16,
	DMX_ATTR_UNKNOWN = 0,
	
	DMX_ATTR_ELEMENT,
	DMX_ATTR_INT32,
	DMX_ATTR_F32,
	DMX_ATTR_BOOL,
	DMX_ATTR_STRING,
	DMX_ATTR_BINARYBLOB,
	DMX_ATTR_TIMESPAN,
	DMX_ATTR_RGBA8,
	DMX_ATTR_VECTOR2D,
	DMX_ATTR_VECTOR3D,
	DMX_ATTR_VECTOR4D,
	DMX_ATTR_QANGLE,
	DMX_ATTR_QUATERNION,
	DMX_ATTR_MATRIX4X4,
	DMX_ATTR_UINT64,
	DMX_ATTR_BYTE,
	
	DMX_ATTR_ELEMENT_ARRAY,
	DMX_ATTR_INT32_ARRAY,
	DMX_ATTR_F32_ARRAY,
	DMX_ATTR_BOOL_ARRAY,
	DMX_ATTR_STRING_ARRAY,
	DMX_ATTR_BINARYBLOB_ARRAY,
	DMX_ATTR_TIMESPAN_ARRAY,
	DMX_ATTR_RGBA8_ARRAY,
	DMX_ATTR_VECTOR2D_ARRAY,
	DMX_ATTR_VECTOR3D_ARRAY,
	DMX_ATTR_VECTOR4D_ARRAY,
	DMX_ATTR_QANGLE_ARRAY,
	DMX_ATTR_QUATERNION_ARRAY,
	DMX_ATTR_MATRIX4X4_ARRAY,
	DMX_ATTR_UINT64_ARRAY,
	DMX_ATTR_BYTE_ARRAY,
	
	// TODO: WHY??? ARE THESE A THING???
	DMX_ATTR_ELEMENT_ARRAY2,
	DMX_ATTR_INT32_ARRAY2,
	DMX_ATTR_F32_ARRAY2,
	DMX_ATTR_BOOL_ARRAY2,
	DMX_ATTR_STRING_ARRAY2,
	DMX_ATTR_BINARYBLOB_ARRAY2,
	DMX_ATTR_TIMESPAN_ARRAY2,
	DMX_ATTR_RGBA8_ARRAY2,
	DMX_ATTR_VECTOR2D_ARRAY2,
	DMX_ATTR_VECTOR3D_ARRAY2,
	DMX_ATTR_VECTOR4D_ARRAY2,
	DMX_ATTR_QANGLE_ARRAY2,
	DMX_ATTR_QUATERNION_ARRAY2,
	DMX_ATTR_MATRIX4X4_ARRAY2,
	DMX_ATTR_UINT64_ARRAY2,
	DMX_ATTR_BYTE_ARRAY2,
};

struct DmxElementId
{
	s32 index;
	char *guid;
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
		char *string; // STRING
		s32 stringIndex; // STRING
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
				char **stringArray; // STRING
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
	char *name;
	DmxAttrValue value;
};

struct DmxPrefixElement
{
	s32 attributeCount;
	DmxAttribute *attributes;
};

struct DmxStringTable
{
	s32 stringCount;
    char **strings;
};

struct DmxElementHeader
{
	char *type;
	char *name;
	u8 guid[16];
};

struct DmxElementBody
{
	s32 attributeCount;
	DmxAttribute *attributes;
};

struct DmxBinary_v9
{
	ReadFileResult file;
	char *header; // "<!-- dmx encoding binary 9 format %s %i -->\n"
	s32 prefixElementCount;
	DmxPrefixElement *prefixElements;
	DmxStringTable stringTable;
	s32 elementCount;
	DmxElementHeader *elementHeaders;
	DmxElementBody *elements;
};

#endif //DMX_H
