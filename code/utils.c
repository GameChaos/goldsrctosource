
static_function inline FileWritingBuffer BufferCreate(Arena *arena, i64 size)
{
	FileWritingBuffer result = {};
	if (size > 0)
	{
		result.size = size;
		result.memory = ArenaAlloc(arena, size);
	}
	return result;
}

static_function inline FileWritingBuffer BufferReset(FileWritingBuffer *buffer)
{
	FileWritingBuffer result = {};
	buffer->usedBytes = 0;
	return result;
}

static_function inline i64 BufferGetSize(FileWritingBuffer buffer)
{
	i64 result = buffer.usedBytes;
	return result;
}

static_function inline i64 BufferGetFreeSpace(FileWritingBuffer buffer)
{
	i64 result = buffer.size - buffer.usedBytes;
	return result;
}

static_function inline void *BufferPushSize(FileWritingBuffer *buffer, i64 size, bool align/* = true*/)
{
	void *result = NULL;
	i64 space = BufferGetFreeSpace(*buffer);
	ASSERT(space > 0);
	if (space > 0)
	{
		result = buffer->memory + buffer->usedBytes;
		buffer->usedBytes += size;
		
		// NOTE(GameChaos): align to 4 bytes
		if (align)
		{
			buffer->usedBytes = ((buffer->usedBytes + 0x3) & (~((i64)0x3)));
		}
	}
	
	return result;
}


static_function inline void *BufferPushData(FileWritingBuffer *buffer, const void *data, i64 dataSize, bool align/* = true*/)
{
	void *result = BufferPushSize(buffer, dataSize, align);
	if (result != NULL)
	{
		Mem_Copy(data, result, GCM_MIN(dataSize, BufferGetFreeSpace(*buffer)));
		result = buffer->memory + buffer->usedBytes;
	}
	return result;
}

static_function inline void *BufferWriteCString(FileWritingBuffer *buffer, const char *string)
{
	ASSERT(buffer);
	ASSERT(string);
	void *result = BufferPushData(buffer, string, strlen(string) + 1, false);
	return result;
}

#define DEFINE_BUFFER_WRITE_TYPE(functionName, type)\
static_function void *functionName(FileWritingBuffer *buffer, type value, bool align)\
{\
void *result = BufferPushData(buffer, &value, sizeof(type), align);\
return result;\
}\

DEFINE_BUFFER_WRITE_TYPE(BufferPushU8, u8)
DEFINE_BUFFER_WRITE_TYPE(BufferPushU16, u16)
DEFINE_BUFFER_WRITE_TYPE(BufferPushU32, u32)
DEFINE_BUFFER_WRITE_TYPE(BufferPushU64, u64)
DEFINE_BUFFER_WRITE_TYPE(BufferPushI8, i8)
DEFINE_BUFFER_WRITE_TYPE(BufferPushI16, i16)
DEFINE_BUFFER_WRITE_TYPE(BufferPushI32, i32)
DEFINE_BUFFER_WRITE_TYPE(BufferPushI64, i64)
DEFINE_BUFFER_WRITE_TYPE(BufferPushF32, f32)
DEFINE_BUFFER_WRITE_TYPE(BufferPushF64, f64)
DEFINE_BUFFER_WRITE_TYPE(BufferPushV2, v2)
DEFINE_BUFFER_WRITE_TYPE(BufferPushV3, v3)
DEFINE_BUFFER_WRITE_TYPE(BufferPushV4, v4)
DEFINE_BUFFER_WRITE_TYPE(BufferPushMat4, mat4)

static_function bool AabbCheck(aabb b1, aabb b2) 
{ 
	bool result = !(b1.maxs.x < b2.mins.x
				   || b1.mins.x > b2.maxs.x
				   || b1.maxs.y < b2.mins.y
				   || b1.mins.y > b2.maxs.y
				   || b1.maxs.z < b2.mins.z
				   || b1.mins.z > b2.maxs.z);
	return result;
}

static_function bool AabbiCheck(aabbi b1, aabbi b2) 
{ 
	bool result = !(b1.maxs.x < b2.mins.x
				   || b1.mins.x > b2.maxs.x
				   || b1.maxs.y < b2.mins.y
				   || b1.mins.y > b2.maxs.y
				   || b1.maxs.z < b2.mins.z
				   || b1.mins.z > b2.maxs.z);
	return result;
}

static_function bool AabbiCheckPoint(aabbi a, v3i b) 
{ 
	bool result = (a.maxs.x >= b.x
				  && a.maxs.y >= b.y
				  && a.maxs.z >= b.z
				  && a.mins.x <= b.x
				  && a.mins.y <= b.y
				  && a.mins.z <= b.z);
	return result;
}

static_function inline v3 LinearInterpolate(v3 vec1, v3 vec2, f32 fraction)
{
	v3 result = v3sub(vec2, vec1);
	result = v3add(v3muls(result, fraction), vec1);
	
	return result;
}

static_function inline v3 GetNonParallelVector(v3 vec)
{
	v3 result = {};
	
	if (!(vec.x == 0 && vec.y == 0 && vec.z == 0))
	{
		v3 temp = {};
		
		// find a vector that is pretty far from vec.
		f32 absX = GCM_ABS(vec.x);
		f32 absY = GCM_ABS(vec.y);
		f32 absZ = GCM_ABS(vec.z);
		
		if (absX < absY)
		{
			temp.x = 1;
		}
		else if (absY < absZ)
		{
			temp.y = 1;
		}
		else
		{
			temp.z = 1;
		}
		
		result = temp;
	}
	
	return result;
}

static_function bool ClipPolygon(Verts *poly, SrcPlane plane)
{
	bool result = true;
	// NOTE(GameChaos): static_persist because struct is way too
	//  big to allocate on the stack
	static_persist Verts newPoly = {};
	newPoly.vertCount = 0;
	
	for (i32 i = 0; i < poly->vertCount; i++)
	{
		u32 nextIndex = (i + 1) % (poly->vertCount);
		v3 point1 = poly->verts[i];
		v3 point2 = poly->verts[nextIndex];
		f32 point1Distance = v3dot(plane.normal, point1) - plane.distance;
		f32 point2Distance = v3dot(plane.normal, point2) - plane.distance;
		bool point1Behind = point1Distance < 0;
		bool point2Behind = point2Distance < 0;
		
		if (point1Behind)
		{
			newPoly.verts[newPoly.vertCount++] = point1;
			ASSERT((i64)newPoly.vertCount < (i64)ARRAYCOUNT(newPoly.verts));
		}
		
		if (newPoly.vertCount >= SRC_MAX_SIDE_VERTS)
		{
			// TODO: error
			result = false;
			break;
		}
		
		// intersect line with plane. only if the 2 points are on either side of the plane
		if ((point1Behind && !point2Behind) || (!point1Behind && point2Behind))
		{
			f32 frac = point1Distance / (point1Distance - point2Distance);
			v3 newPoint = LinearInterpolate(point1, point2, frac);
			newPoly.verts[newPoly.vertCount++] = newPoint;
			ASSERT((i64)newPoly.vertCount < (i64)ARRAYCOUNT(newPoly.verts));
		}
		
		if (newPoly.vertCount >= SRC_MAX_SIDE_VERTS)
		{
			// TODO: error
			result = false;
			break;
		}
	}
	// check if we managed to clip anything
	if (result)
	{
		Mem_Copy(&newPoly, poly, sizeof(*poly));
	}
	
	return result;
}

#define NORMAL_EPSILON 0.00001f
#define DIST_EPSILON 0.01f
static_function bool PlaneEquals(SrcPlane plane, v3 normal, f32 dist)
{
	f32 normEpsilon = NORMAL_EPSILON;
	f32 distEpsilon = DIST_EPSILON;
	bool result = false;
	if (GCM_ABS(plane.normal.e[0] - normal.e[0]) < normEpsilon
		&& GCM_ABS(plane.normal.e[1] - normal.e[1]) < normEpsilon
		&& GCM_ABS(plane.normal.e[2] - normal.e[2]) < normEpsilon
		&& GCM_ABS(plane.distance - dist) < distEpsilon)
	{
		result = true;
	}
	return result;
}

static_function bool VecNearlyEquals(v3 a, v3 b, f32 epsilon/* = NORMAL_EPSILON*/)
{
	bool result = false;
	if (GCM_ABS(a.x - b.x) < epsilon
		&& GCM_ABS(a.y - b.y) < epsilon
		&& GCM_ABS(a.z - b.z) < epsilon)
	{
		result = true;
	}
	return result;
}

#define PARENT_TO_INDEX(parent) ((parent) < 0 ? -(parent) - 1 : (parent))

static_function f32 PolygonArea(Verts *poly, v3 normal)
{
	f32 result = 0;
	
	v3 total = {};
	for (i32 v = 0; v < poly->vertCount; v++)
	{
		v3 vert1 = poly->verts[v];
		v3 vert2 = poly->verts[(v + 1) % poly->vertCount];
		
		v3 cross = v3cross(vert1, vert2);
		total = v3add(total, cross);
	}
	result = v3dot(total, normal);
	result = GCM_ABS(result * 0.5f);
	return result;
}

static_function bool MakePolygon(SrcPlane *planes, i32 planeCount, i32 planeIndex, Verts *out)
{
	bool result = false;
	SrcPlane plane = planes[planeIndex];
	f32 normalLen = v3len(plane.normal);
	if (normalLen < 0.9 || normalLen > 1.1)
	{
		// TODO: normalise the normal ourselves instead of just up and failing?
		Warning("Plane %i normal isn't a unit vector!\n", planeIndex);
		return result;
	}
	// make a vector that isn't parallel to the plane's normal
	v3 nonParallel = GetNonParallelVector(plane.normal);
	
	f32 spread = SRC_MAP_SIZE;
	v3 tangentX = v3cross(plane.normal, nonParallel);
	tangentX = v3normalise(tangentX);
	v3 tangentY = v3cross(plane.normal, tangentX);
	tangentY = v3normalise(tangentY);
	
	v3 pointOnPlane = v3muls(plane.normal, plane.distance);
	static_assert(SRC_MAX_SIDE_VERTS > 4, "");
	// generate an initial polygon that's bigger than the map and is in the same direction of the plane.
#if 1
	out->verts[out->vertCount++] = v3add(v3muls(v3add(v3negate(tangentX), v3negate(tangentY)), spread), pointOnPlane);
	out->verts[out->vertCount++] = v3add(v3muls(v3add(v3negate(tangentX),           tangentY), spread), pointOnPlane);
	out->verts[out->vertCount++] = v3add(v3muls(v3add(          tangentX,           tangentY), spread), pointOnPlane);
	out->verts[out->vertCount++] = v3add(v3muls(v3add(          tangentX, v3negate(tangentY)), spread), pointOnPlane);
#else
	for (i32 i = 0; i < 4; i++)
	{
		i32 negX = -(i == 0 || i == 1);
		i32 negY = -(i == 0 || i == 3);
		v3 vert = v3add(v3muls(tangentX, negX), v3muls(tangentY, negY));
		vert = v3muls(vert, spread);
		vert = v3add(vert, pointOnPlane);
		out->verts[out->vertCount++] = vert;
	}
#endif
	ASSERT((i64)out->vertCount < (i64)ARRAYCOUNT(out->verts));
	
	for (i32 planeInd = 0; planeInd < planeCount; planeInd++)
	{
		if (planeInd != planeIndex)
		{
			ClipPolygon(out, planes[planeInd]);
		}
	}
	
	// NOTE(GameChaos): remove duplicate vertices
	for (i32 ind1 = 0; ind1 < out->vertCount; ind1++)
	{
		i32 ind2 = (ind1 + 1) % out->vertCount;
		v3 vert1 = out->verts[ind1];
		v3 vert2 = out->verts[ind2];
		if (v3equals(vert1, vert2))
		{
			for (i32 v = ind2; v < out->vertCount - 1; v++)
			{
				out->verts[v] = out->verts[v + 1];
			}
			out->vertCount--;
		}
#if 0
		// this removes all duplicate vertices whether they're connected or not, BAD!
		for (i32 ind2 = 0; ind2 < out->vertCount; ind2++)
		{
			v3 vert2 = out->verts[ind2];
			if (ind1 != ind2 && VecNearlyEquals(vert1, vert2, 0.01f))
			{
				// NOTE(GameChaos): remove duplicate vert
				for (i32 v = ind2; v < out->vertCount - 1; v++)
				{
					out->verts[v] = out->verts[v + 1];
				}
				out->vertCount--;
				ind2--;
			}
		}
#endif
	}
	
	result = out->vertCount >= 3;
	if (result)
	{
		f32 area = PolygonArea(out, plane.normal);
		if (area < 0.01f)
		{
			result = false;
		}
	}
	return result;
}

static_function i32 Add_(u8 *array, i32 *count, void *data, i32 elementSize, i32 maxElements, const char *arrayName)
{
	i32 result = 0;
	if (*count < maxElements)
	{
		for (i32 i = 0; i < *count; i++)
		{
			if (Mem_Compare(&array[i * elementSize], data, elementSize))
			{
				result = i;
				return result;
			}
		}
		result = *count;
		Mem_Copy(data, array + (*count) * elementSize, elementSize);
		(*count)++;
	}
	else
	{
		char error[256];
		Format(error, sizeof(error), "Exceeded maximum amount of %s!", arrayName);
		FatalError(error);
	}
	return result;
}

static_function i32 AddSimple_(u8 *array, i32 *count, void *data, i32 elementSize, i32 maxElements, const char *arrayName)
{
	i32 result = 0;
	if (*count < maxElements)
	{
		result = *count;
		Mem_Copy(data, array + (*count) * elementSize, elementSize);
		(*count)++;
	}
	else
	{
		char error[256];
		Format(error, sizeof(error), "Exceeded maximum amount of %s!", arrayName);
		FatalError(error);
	}
	return result;
}

#define AddLeaf(array, count, data) AddSimple_((u8 *)(array), (count), &(data), sizeof(data), SRC_MAX_MAP_LEAFS, "leaves")
#define AddBrushSide(array, count, data) AddSimple_((u8 *)(array), (count), &(data), sizeof(data), SRC_MAX_MAP_BRUSHSIDES, "brush sides")
#define AddPlane(array, count, data) Add_((u8 *)(array), (count), &(data), sizeof(data), SRC_MAX_MAP_PLANES, "planes")

static_function bool IsWhiteSpace(char c)
{
	bool result = c == ' ' || c == '\t'
		|| c == '\v' || c == '\n'
		|| c == '\r' || c == '\f';
	return result;
}

static_function v3 SnapVector(v3 normal)
{
	v3 result = normal;
	for (i32 i = 0; i < 3 ; i++)
	{
		if (GCM_ABS(result.e[i] - 1) < NORMAL_EPSILON)
		{
			result = (v3){};
			result.e[i] = 1;
			break;
		}
		if (GCM_ABS(result.e[i] + 1) < NORMAL_EPSILON)
		{
			result = (v3){};
			result.e[i] = -1;
			break;
		}
	}
	return result;
}

static_function i32 i32pow(i32 base, i32 power)
{
	i32 result = base;
	ASSERT(GCM_ABS(base) < 2 || power < 32);
	if (power == 0)
	{
		result = 1;
	}
	else
	{
		for (i32 i = 0; i < power; i++)
		{
			result *= base;
		}
	}
    return result;
}

static_function i32 convert(char str[], i32 size)
{
    i32 number = 0;
    for (i32 i = 0; i < size; ++i)
	{
        number += (str[i] - '0') * i32pow(10, (size - i - 1));
    }
    return number;
}

static_function i32 pow10_(i32 radix)
{
    i32 r = 1;
    for (i32 i = 0; i < radix; i++)
	{
        r *= 10;
	}
    return r;
}

// NOTE(GameChaos): TEMPORARY https://stackoverflow.com/a/52706534
static_function f32 StrToF32(str str)
{
    // convert to string_without_decimal
    char str_without_decimal[64];
    i32 c = 0;
    for (i32 i = 0; i < str.length; i++)
    {
        if (str.data[i] >= '0' && str.data[i] <= '9')
		{
            str_without_decimal[c] = str.data[i];
            c++;
        }
    }
    str_without_decimal[c] = '\0';
	
    //adjust size if dot present or not.  If no dot present => size = c
	i32 size = (i32)str.length;
    size = (size != c) ? size - 1 : size;      //size = 5 = 6-1 since dot is present
	
	//convert to decimal
	i32 decimal = convert(str_without_decimal, size);  //decimal = 12345
	
	//get divisor
	i32 i;
	for (i = (i32)str.length; i >= 0; i--)
	{
		if (str.data[i] == '.')
		{
			break;
		}
	}
	i32 divisor = pow10_(size - i);     //divisor = 10;
	f32 result = (f32)decimal / (f32)divisor;
	return result;  // result = 12345 /10
}

static_function inline const char *StringToS32(const char *string, i32 *out)
{
	const char *result = NULL;
	if (string)
	{
		while (string[0] == ' '
			   || string[0] == '\t'
			   || string[0] == '\v'
			   || string[0] == '\n'
			   || string[0] == '\r'
			   || string[0] == '\f')
		{
			string++;
		}
		
		bool negative = false;
		if (*string == '-')
		{
			negative = true;
			string++;
		}
		
		i64 value = 0;
		i64 magnitude = 1;
		for (const char *c = string;
			 *c >= '0' && *c <= '9';
			 c++)
		{
			value *= 10;
			value += (i64)(*c - '0');
			result = c + 1;
			if (value > (i64)I32_MAX + 1)
			{
				break;
			}
		}
		
		if (negative)
		{
			value = -value;
		}
		
		if (value > I32_MAX)
		{
			*out = I32_MAX;
		}
		else if (value < I32_MIN)
		{
			*out = I32_MIN;
		}
		if (result)
		{
			*out = (i32)value;
		}
		
	}
	return result;
}
