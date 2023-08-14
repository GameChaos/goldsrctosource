
inline FileWritingBuffer BufferCreate(Arena *arena, s64 size)
{
	FileWritingBuffer result = {};
	if (size > 0)
	{
		result.size = size;
		result.memory = (u8 *)ArenaAlloc(arena, size);
	}
	return result;
}

inline FileWritingBuffer BufferReset(FileWritingBuffer *buffer)
{
	FileWritingBuffer result = {};
	buffer->usedBytes = 0;
	return result;
}

inline s64 BufferGetSize(FileWritingBuffer buffer)
{
	s64 result = buffer.usedBytes;
	return result;
}

inline s64 BufferGetFreeSpace(FileWritingBuffer buffer)
{
	s64 result = buffer.size - buffer.usedBytes;
	return result;
}

inline void *BufferPushSize(FileWritingBuffer *buffer, s64 size, b32 align = true)
{
	void *result = NULL;
	s64 space = BufferGetFreeSpace(*buffer);
	ASSERT(space > 0);
	if (space > 0)
	{
		result = buffer->memory + buffer->usedBytes;
		buffer->usedBytes += size;
		
		// NOTE(GameChaos): align to 4 bytes
		if (align)
		{
			buffer->usedBytes = ((buffer->usedBytes + 0x3) & (~((s64)0x3)));
		}
	}
	
	return result;
}

inline void *BufferPushData(FileWritingBuffer *buffer, void *data, s64 dataSize, b32 align = true)
{
	void *result = BufferPushSize(buffer, dataSize, align);
	if (result != NULL)
	{
		Mem_Copy(data, result, dataSize, BufferGetFreeSpace(*buffer));
		result = buffer->memory + buffer->usedBytes;
	}
	return result;
}

internal b32 AabbCheck(aabb b1, aabb b2) 
{ 
	b32 result = !(b1.maxs.x < b2.mins.x
				   || b1.mins.x > b2.maxs.x
				   || b1.maxs.y < b2.mins.y
				   || b1.mins.y > b2.maxs.y
				   || b1.maxs.z < b2.mins.z
				   || b1.mins.z > b2.maxs.z);
	return result;
}

internal b32 AabbiCheck(aabbi b1, aabbi b2) 
{ 
	b32 result = !(b1.maxs.x < b2.mins.x
				   || b1.mins.x > b2.maxs.x
				   || b1.maxs.y < b2.mins.y
				   || b1.mins.y > b2.maxs.y
				   || b1.maxs.z < b2.mins.z
				   || b1.mins.z > b2.maxs.z);
	return result;
}

internal b32 AabbiCheckPoint(aabbi a, v3i b) 
{ 
	b32 result = (a.maxs.x >= b.x
				  && a.maxs.y >= b.y
				  && a.maxs.z >= b.z
				  && a.mins.x <= b.x
				  && a.mins.y <= b.y
				  && a.mins.z <= b.z);
	return result;
}

inline v3 LinearInterpolate(v3 vec1, v3 vec2, f32 fraction)
{
	v3 result = vec2 - vec1;
	result = result * fraction + vec1;
	
	return result;
}

inline v3 GetNonParallelVector(v3 vec)
{
	v3 result = {};
	
	if (!(vec.x == 0 && vec.y == 0 && vec.z == 0))
	{
		v3 temp = {};
		
		// find a vector that is pretty far from vec.
		f32 absX = HMM_ABS(vec.x);
		f32 absY = HMM_ABS(vec.y);
		f32 absZ = HMM_ABS(vec.z);
		
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

internal b32 ClipPolygon(Verts *poly, SrcPlane plane)
{
	b32 result = true;
	local_persist Verts newPoly = {};
	newPoly.vertCount = 0;
	
	for (s32 i = 0; i < poly->vertCount; i++)
	{
		u32 nextIndex = (i + 1) % (poly->vertCount);
		v3 point1 = poly->verts[i];
		v3 point2 = poly->verts[nextIndex];
		f32 point1Distance = Dot(plane.normal, point1) - plane.distance;
		f32 point2Distance = Dot(plane.normal, point2) - plane.distance;
		b32 point1Behind = point1Distance < 0;
		b32 point2Behind = point2Distance < 0;
		
		if (point1Behind)
		{
			newPoly.verts[newPoly.vertCount++] = point1;
			ASSERT(newPoly.vertCount < ARRAYCOUNT(newPoly.verts));
		}
		
		if (newPoly.vertCount >= SRC_MAX_SIDE_VERTS)
		{
			// TODO: error
			result = false;
			break;
		}
		
		// intersect line with plane. only if the 2 points are on either side of the plane
		if (point1Behind && !point2Behind || !point1Behind && point2Behind)
		{
			f32 frac = point1Distance / (point1Distance - point2Distance);
			v3 newPoint = LinearInterpolate(point1, point2, frac);
			newPoly.verts[newPoly.vertCount++] = newPoint;
			ASSERT(newPoly.vertCount < ARRAYCOUNT(newPoly.verts));
		}
		
		if (newPoly.vertCount >= SRC_MAX_SIDE_VERTS)
		{
			// TODO: error
			result = false;
			break;
		}
	}
	// check if we managed to clip anything
	if (newPoly.verts && result)
	{
		Mem_Copy(&newPoly, poly, sizeof(newPoly), sizeof(*poly));
	}
	
	return result;
}

#define NORMAL_EPSILON 0.00001f
#define DIST_EPSILON 0.01f
internal b32 PlaneEquals(SrcPlane plane, v3 normal, f32 dist,
						 f32 normEpsilon = NORMAL_EPSILON, f32 distEpsilon = DIST_EPSILON)
{
	b32 result = false;
	if (HMM_ABS(plane.normal[0] - normal[0]) < normEpsilon
		&& HMM_ABS(plane.normal[1] - normal[1]) < normEpsilon
		&& HMM_ABS(plane.normal[2] - normal[2]) < normEpsilon
		&& HMM_ABS(plane.distance - dist) < distEpsilon)
	{
		result = true;
	}
	return result;
}

internal b32 VecNearlyEquals(v3 a, v3 b, f32 epsilon = NORMAL_EPSILON)
{
	b32 result = false;
	if (HMM_ABS(a[0] - b[0]) < epsilon
		&& HMM_ABS(a[1] - b[1]) < epsilon
		&& HMM_ABS(a[2] - b[2]) < epsilon)
	{
		result = true;
	}
	return result;
}

#define PARENT_TO_INDEX(parent) ((parent) < 0 ? -(parent) - 1 : (parent))

internal f32 PolygonArea(Verts *poly, v3 normal)
{
	f32 result = 0;
	
	v3 total = {};
	for (s32 v = 0; v < poly->vertCount; v++)
	{
		v3 vert1 = poly->verts[v];
		v3 vert2 = poly->verts[(v + 1) % poly->vertCount];
		
		v3 cross = Cross(vert1, vert2);
		total += cross;
	}
	result = Dot(total, normal);
	result = HMM_ABS(result * 0.5f);
	return result;
}

internal b32 MakePolygon(SrcPlane *planes, s32 planeCount, s32 planeIndex, Verts *out)
{
	b32 result = false;
	SrcPlane plane = planes[planeIndex];
	f32 normalLen = Length(plane.normal);
	if (normalLen < 0.9 || normalLen > 1.1)
	{
		// TODO: normalise the normal ourselves instead of just up and failing?
		Warning("Plane %i normal isn't a unit vector!\n", planeIndex);
		return result;
	}
	// make a vector that isn't parallel to the plane's normal
	v3 nonParallel = GetNonParallelVector(plane.normal);
	
	f32 spread = SRC_MAP_SIZE;
	v3 tangentX = Cross(plane.normal, nonParallel);
	tangentX = Normalize(tangentX);
	v3 tangentY = Cross(plane.normal, tangentX);
	tangentY = Normalize(tangentY);
	
	v3 pointOnPlane = plane.normal * plane.distance;
	static_assert(SRC_MAX_SIDE_VERTS > 4, "");
	// generate an initial polygon that's bigger than the map and is in the same direction of the plane.
	out->verts[out->vertCount++] = (-tangentX + -tangentY) * spread + pointOnPlane;
	out->verts[out->vertCount++] = (-tangentX +  tangentY) * spread + pointOnPlane;
	out->verts[out->vertCount++] = ( tangentX +  tangentY) * spread + pointOnPlane;
	out->verts[out->vertCount++] = ( tangentX + -tangentY) * spread + pointOnPlane;
	ASSERT(out->vertCount < ARRAYCOUNT(out->verts));
	
	for (s32 planeInd = 0; planeInd < planeCount; planeInd++)
	{
		if (planeInd != planeIndex)
		{
			ClipPolygon(out, planes[planeInd]);
		}
	}
	
	// NOTE(GameChaos): remove duplicate vertices
	for (s32 ind1 = 0; ind1 < out->vertCount; ind1++)
	{
		v3 vert1 = out->verts[ind1];
		for (s32 ind2 = 0; ind2 < out->vertCount; ind2++)
		{
			v3 vert2 = out->verts[ind2];
			if (ind1 != ind2 && VecNearlyEquals(vert1, vert2, 0.01f))
			{
				// NOTE(GameChaos): remove duplicate vert
				for (s32 v = ind2; v < out->vertCount - 1; v++)
				{
					out->verts[v] = out->verts[v + 1];
				}
				out->vertCount--;
				ind2--;
			}
		}
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

internal s32 Add_(u8 *array, s32 *count, void *data, s32 elementSize, s32 maxElements, char *arrayName)
{
	s32 result = 0;
	if (*count < maxElements)
	{
		for (s32 i = 0; i < *count; i++)
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

internal s32 AddSimple_(u8 *array, s32 *count, void *data, s32 elementSize, s32 maxElements, char *arrayName)
{
	s32 result = 0;
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

internal b32 IsWhiteSpace(char c)
{
	b32 result = c == ' ' || c == '\t'
		|| c == '\v' || c == '\n'
		|| c == '\r' || c == '\f';
	return result;
}

internal v3 SnapVector(v3 normal)
{
	v3 result = normal;
	for (s32 i = 0; i < 3 ; i++)
	{
		if (HMM_ABS(result[i] - 1) < NORMAL_EPSILON)
		{
			result = {};
			result[i] = 1;
			break;
		}
		if (HMM_ABS(result[i] + 1) < NORMAL_EPSILON)
		{
			result = {};
			result[i] = -1;
			break;
		}
	}
	return result;
}

internal s32 s32pow(s32 base, s32 power)
{
	s32 result = base;
	ASSERT(HMM_ABS(base) < 2 || power < 32);
	if (power == 0)
	{
		result = 1;
	}
	else
	{
		for (s32 i = 0; i < power; i++)
		{
			result *= base;
		}
	}
    return result;
}

internal s32 convert(char str[], s32 size)
{
    s32 number = 0;
    for (s32 i = 0; i < size; ++i)
	{
        number += (str[i] - '0') * s32pow(10, (size - i - 1));
    }
    return number;
}

internal s32 pow10_(s32 radix)
{
    s32 r = 1;
    for (s32 i = 0; i < radix; i++)
	{
        r *= 10;
	}
    return r;
}

// NOTE(GameChaos): TEMPORARY https://stackoverflow.com/a/52706534
internal f32 StrToF32(str str)
{
    // convert to string_without_decimal
    char str_without_decimal[64];
    s32 c = 0;
    for (s32 i = 0; i < str.length; i++)
    {
        if (str.data[i] >= '0' && str.data[i] <= '9')
		{
            str_without_decimal[c] = str.data[i];
            c++;
        }
    }
    str_without_decimal[c] = '\0';
	
    //adjust size if dot present or not.  If no dot present => size = c
	s32 size = (s32)str.length;
    size = (size != c) ? size - 1 : size;      //size = 5 = 6-1 since dot is present
	
	//convert to decimal
	s32 decimal = convert(str_without_decimal, size);  //decimal = 12345
	
	//get divisor
	s32 i;
	for (i = (s32)str.length; i >= 0; i--)
	{
		if (str.data[i] == '.')
		{
			break;
		}
	}
	s32 divisor = pow10_(size - i);     //divisor = 10;
	f32 result = (f32)decimal / (f32)divisor;
	return result;  // result = 12345 /10
}

inline StringToNumResult StringToS32(char *string, s32 *out)
{
	StringToNumResult result = STRINGTONUM_ERR_FAILED;
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
		
		b32 negative = false;
		if (*string == '-')
		{
			negative = true;
			*string++;
		}
		
		s64 value = 0;
		s64 magnitude = 1;
		for (char *c = string;
			 *c >= '0' && *c <= '9';
			 c++)
		{
			value *= 10;
			value += (s64)(*c - '0');
			result = STRINGTONUM_SUCCESS;
			if (value > (s64)S32_MAX + 1)
			{
				break;
			}
		}
		
		if (negative)
		{
			value = -value;
		}
		
		if (value > S32_MAX)
		{
			*out = S32_MAX;
			result = STRINGTONUM_ERR_NUM_TOO_BIG;
		}
		else if (value < S32_MIN)
		{
			*out = S32_MIN;
			result = STRINGTONUM_ERR_NUM_TOO_BIG;
		}
		if (result == STRINGTONUM_SUCCESS)
		{
			*out = (s32)value;
		}
		
	}
	return result;
}
