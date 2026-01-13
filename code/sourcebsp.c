#include "sourcebsp.h"

static_global const char *g_srcLumpNames[] = {
	"LUMP_ENTITIES",
	"LUMP_PLANES",
	"LUMP_TEXDATA",
	"LUMP_VERTEXES",
	"LUMP_VISIBILITY",
	"LUMP_NODES",
	"LUMP_TEXINFO",
	"LUMP_FACES",
	"LUMP_LIGHTING",
	"LUMP_OCCLUSION",
	"LUMP_LEAFS",
	"LUMP_FACEIDS",
	"LUMP_EDGES",
	"LUMP_SURFEDGES",
	"LUMP_MODELS",
	"LUMP_WORLDLIGHTS",
	"LUMP_LEAFFACES",
	"LUMP_LEAFBRUSHES",
	"LUMP_BRUSHES",
	"LUMP_BRUSHSIDES",
	"LUMP_AREAS",
	"LUMP_AREAPORTALS",
	"LUMP_FACEBRUSHES",
	"LUMP_FACEBRUSHLIST",
	"LUMP_UNUSED1",
	"LUMP_UNUSED2",
	"LUMP_DISPINFO",
	"LUMP_ORIGINALFACES",
	"LUMP_PHYSDISP",
	"LUMP_PHYSCOLLIDE",
	"LUMP_VERTNORMALS",
	"LUMP_VERTNORMALINDICES",
	"LUMP_DISP_LIGHTMAP_ALPHAS",
	"LUMP_DISP_VERTS",
	"LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS",
	"LUMP_GAME_LUMP",
	"LUMP_LEAFWATERDATA",
	"LUMP_PRIMITIVES",
	"LUMP_PRIMVERTS",
	"LUMP_PRIMINDICES",
	"LUMP_PAKFILE",
	"LUMP_CLIPPORTALVERTS",
	"LUMP_CUBEMAPS",
	"LUMP_TEXDATA_STRING_DATA",
	"LUMP_TEXDATA_STRING_TABLE",
	"LUMP_OVERLAYS",
	"LUMP_LEAFMINDISTTOWATER",
	"LUMP_FACE_MACRO_TEXTURE_INFO",
	"LUMP_DISP_TRIS",
	"LUMP_PROP_BLOB",
	"LUMP_WATEROVERLAYS",
	"LUMP_LEAF_AMBIENT_INDEX_HDR",
	"LUMP_LEAF_AMBIENT_INDEX",
	"LUMP_LIGHTING_HDR",
	"LUMP_WORLDLIGHTS_HDR",
	"LUMP_LEAF_AMBIENT_LIGHTING_HDR",
	"LUMP_LEAF_AMBIENT_LIGHTING",
	"LUMP_XZIPPAKFILE",
	"LUMP_FACES_HDR",
	"LUMP_MAP_FLAGS",
	"LUMP_OVERLAY_FADES",
	"LUMP_OVERLAY_SYSTEM_LEVELS",
	"LUMP_PHYSLEVEL",
	"LUMP_DISP_MULTIBLEND"
};

static_function i32 SrcGetPlaneType(v3 normal)
{
	i32 type = -1;
	for (int i = 0; i < 3; i++)
	{
		if (normal.e[i] == 1.0f || normal.e[i] == -1.0f)
		{
			type = i;
			break;
		}
	}
	
	if (type == -1)
	{
		f32 x = GCM_ABS(normal.x);
		f32 y = GCM_ABS(normal.y);
		f32 z = GCM_ABS(normal.z);
		
		if (x >= y && x >= z)
		{
			type = SRC_PLANE_ANYX;
		}
		else if (y >= x && y >= z)
		{
			type = SRC_PLANE_ANYY;
		}
		else
		{
			type = SRC_PLANE_ANYZ;
		}
	}
	return type;
}

static_function Rgbe8888 SrcLinearToRgbe8888(v3 colour)
{
	f32 max = GCM_MAX(GCM_MAX(colour.r, colour.g), colour.b);
	i32 exponent = -128;
	if (max > 0)
	{
		exponent = (i32)floorf(log2f(max)) + 1;
		exponent = (i8)GCM_CLAMP(-128, exponent, 127);
		f32 scale = powf(2.0f, -(f32)exponent);
		colour = v3muls(colour, scale);
		colour = v3muls(colour, 255.0f);
	}
	for (i32 i = 0; i < 3; i++)
	{
		colour.e[i] = GCM_CLAMP(0, colour.e[i], 255);
	}
	
	Rgbe8888 result = {
		(u8)colour.x,
		(u8)colour.y,
		(u8)colour.z,
		(i8)exponent,
	};
	
	return result;
}

static_function v3 SrcRgbe8888ToLinear(Rgbe8888 colour)
{
	f32 mul = powf(2, colour.exponent);
	v3 result = {
		(f32)colour.r * mul,
		(f32)colour.g * mul,
		(f32)colour.b * mul,
	};
	result = v3muls(result, 1.0f / 255.0f);
	
	return result;
}
