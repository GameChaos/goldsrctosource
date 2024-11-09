
#include "vmap.h"

static_function bool VmapFromGoldsrc(Arena *arena, Arena *tempArena, GsrcMapData *mapData, char *outputPath, GamePaths *paths)
{
	bool result = false;
	
	Vmap vmap = {};
	
	
	
	Dmx dmx = DmxCreate(arena, "vmap", 35);
	
	// prefix attributes
	{
		DmxElement *prefixElement = DmxAddElement(&dmx.prefix, NULL, STR(""), STR(""), arena);
		const char *imgData = "P3\n2 2\n0 0 0\n0 0 0\n0 0 0\n0 0 0";
		i64 imgBytes = strlen(imgData);
		DmxAddAttributeBinary(prefixElement, STR("asset_preview_thumbnail"), (void *)imgData, imgBytes);
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
			//DmxAddAttributeArrayString(&dmx, mapRootElement, STR("variableTargetKeys"), NULL, 0);
			//DmxAddAttributeArrayString(&dmx, mapRootElement, STR("variableNames"), NULL, 0);
		}
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
	DmxExportBinary(outputPath, arena, dmx);
	
	return result;
}