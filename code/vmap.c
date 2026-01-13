
#include "vmap.h"

static_function DmxElement *VmapCreatePrefix(Dmx *dmx, Arena *arena)
{
	DmxElement *result = DmxAddElement(&dmx->prefix, NULL, STR(""), STR(""), arena);
	const char *imgData = "P3\n2 2\n0 0 0\n0 0 0\n0 0 0\n0 0 0";
	i64 imgBytes = strlen(imgData);
	DmxAddAttributeBinary(result, STR("asset_preview_thumbnail"), (void *)imgData, imgBytes);
	DmxAddAttributeString(result, STR("asset_preview_thumbnail_format"), STR("ppm"));
	DmxAddAttribute(result, STR("map_asset_references"), DMX_ATTR_STRING_ARRAY);
	DmxAddAttributeBool(result, STR("m_bIsCordoning"), false);
	DmxAddAttributeBool(result, STR("m_bCordonsVisible"), false);
	return result;
}

static_function DmxElement *VmapCreateCMapRootElement(Dmx *dmx, Arena *arena)
{
	DmxElement *result = DmxAddElement(&dmx->body, NULL, STR(""), STR("CMapRootElement"), arena);
	DmxAddAttributeBool(result, STR("isprefab"), false);
	DmxAddAttributeInt(result, STR("editorbuild"), 9820);
	DmxAddAttributeInt(result, STR("editorversion"), 400);
	DmxAddAttributeString(result, STR("itemFile"), STR(""));
	return result;
}

static_function DmxElement *VmapCreateCStoredCamera(Dmx *dmx, DmxElement *mapRootElement, str name, Arena *arena)
{
	DmxElement *result = DmxAddElement(&dmx->body, mapRootElement, name, STR("CStoredCamera"), arena);
	DmxAddAttributeV3(result, STR("position"), (v3){0, -1000, -1000});
	DmxAddAttributeV3(result, STR("lookat"), (v3){0, -999.2929077148f, 999.2929077148f});
	return result;
}

static_function DmxElement *VmapCreateCStoredCameras(Dmx *dmx, DmxElement *mapRootElement, str name, Arena *arena)
{
	DmxElement *result = DmxAddElement(&dmx->body, mapRootElement, name, STR("CStoredCamera"), arena);
	DmxAddAttributeInt(result, STR("activecamera"), -1);
	DmxAddAttributeArrayElementId(result, STR("cameras"), NULL, 0);
	return result;
}

static_function DmxElement *VmapCreateCMapWorld(Dmx *dmx, DmxElement *mapRootElement, Arena *arena)
{
	DmxElement *result = DmxAddElement(&dmx->body, mapRootElement, STR("world"), STR("CMapWorld"), arena);
	// TODO: what the hell is nodeID
	DmxAddAttributeInt(result, STR("nodeID"), 1);
	// TODO: what the hell is referenceID
	DmxAddAttributeU64(result, STR("referenceID"), 0);
	DmxAddAttributeArrayElementId(result, STR("children"), NULL, 0);
	DmxAddAttributeArrayString(result, STR("variableTargetKeys"), NULL, 0);
	DmxAddAttributeArrayString(result, STR("variableNames"), NULL, 0);
	DmxAddAttributeArrayElementId(result, STR("connectionsData"), NULL, 0);
	
	DmxAddAttributeInt(result, STR("nextDecalID"), 0);
	DmxAddAttributeBool(result, STR("fixupEntityNames"), true);
	DmxAddAttributeString(result, STR("mapUsageType"), STR("standard"));
	
	// TODO: these are base attributes, seem to be used on many element types
	DmxAddAttributeV3(result, STR("origin"), (v3){0, 0, 0});
	DmxAddAttributeQAngle(result, STR("angles"), (v3){0, 0, 0});
	DmxAddAttributeV3(result, STR("scales"), (v3){1, 1, 1});
	DmxAddAttributeBool(result, STR("transformLocked"), false);
	DmxAddAttributeBool(result, STR("force_hidden"), false);
	DmxAddAttributeBool(result, STR("editorOnly"), false);
	return result;
}

static_function bool VmapFromGoldsrc(Arena *arena, Arena *tempArena, GsrcMapData *mapData, char *outputPath, GamePaths *paths)
{
	(void)tempArena;
	(void)mapData;
	(void)paths;
	bool result = false;
	
	
	
	Dmx dmx = DmxCreate(arena, "vmap", 35);
	
	
	VmapCreatePrefix(&dmx, arena);
	
	{
		DmxElement *mapRootElement = VmapCreateCMapRootElement(&dmx, arena);
		VmapCreateCStoredCamera(&dmx, mapRootElement, STR("defaultcamera"), arena);
		VmapCreateCStoredCameras(&dmx, mapRootElement, STR("3dcameras"), arena);
		DmxElement *mapWorld = VmapCreateCMapWorld(&dmx, mapRootElement, arena);
	}
	
#if 1
	// NOTE(GameChaos): debug print
	for (i32 elemInd = 0; elemInd < dmx.prefix.count; elemInd++)
	{
		DmxElement *elem = &dmx.prefix.elements[elemInd];
		
		Print("%s: %s\n", elem->type, elem->name);
		for (i32 attrInd = 0; attrInd < elem->attributeCount; attrInd++)
		{
			DmxAttribute *attr = &elem->attributes[attrInd];
			
			Print("\t%s: %i\n", attr->name, attr->value.type);
		}
	}
	
	for (i32 elemInd = 0; elemInd < dmx.body.count; elemInd++)
	{
		DmxElement *elem = &dmx.body.elements[elemInd];
		
		Print("%s: %s\n", elem->type, elem->name);
		for (i32 attrInd = 0; attrInd < elem->attributeCount; attrInd++)
		{
			DmxAttribute *attr = &elem->attributes[attrInd];
			
			Print("\t%s: %i\n", attr->name, attr->value.type);
		}
	}
#endif
	
	DmxDebugPrint(dmx);
	DmxExportBinary(outputPath, arena, dmx);
	
	return result;
}
