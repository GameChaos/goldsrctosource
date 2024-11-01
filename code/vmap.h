/* date = December 13th 2023 0:45 pm */

#ifndef VMAP_TYPES_H
#define VMAP_TYPES_H

struct EditGameClassProps
{
	s32 propertyCount;
	DmxAttribute *properties;
};

dmx_serialise struct DmePlugList
{
	dmx_serialise_array(char *, names);
	dmx_serialise_array(s32, dataTypes);
	dmx_serialise_array(s32, plugTypes);
	dmx_serialise_array(char *, descriptions);
};

dmx_serialise struct CStoredCamera
{
	v3 position;
	v3 lookat;
};

dmx_serialise struct CStoredCameras
{
	s32 activecamera;
	dmx_serialise_array(CStoredCamera, cameras);
};

dmx_serialise struct CMapEntity
{
	dmx_serialise_array(CMapEntity, children);
	dmx_serialise_array(void, variableTargetKeys);
	dmx_serialise_array(void, variableNames);
	dmx_serialise_array(void, connectionsData);
	EditGameClassProps dmx_function(DmxSerialiseEntProps) entity_properties;
	v3 hitNormal;
	bool isProceduralEntity;
	v3 origin;
	QAngle angles;
	v3 scales;
	bool transformLocked;
	bool force_hidden;
	bool editorOnly;
};

dmx_serialise struct CMapGroup
{
	dmx_serialise_array(CMapEntity, children);
	dmx_serialise_array(char *, variableTargetKeys);
	dmx_serialise_array(char *, variableNames);
	v3 origin;
	QAngle angles;
	v3 scales;
	bool transformLocked;
	bool force_hidden;
	bool editorOnly;
};

dmx_serialise struct CVisibilityMgr
{
	dmx_serialise_array(CMapEntity, children);
	dmx_serialise_array(char *, variableTargetKeys);
	dmx_serialise_array(char *, variableNames);
	dmx_serialise_array(CMapGroup, nodes);
	v3 origin;
	QAngle angles;
	v3 scales;
	bool transformLocked;
	bool force_hidden;
	bool editorOnly;
};

dmx_serialise struct CMapVariableSet
{
	dmx_serialise_array(char *, variableNames);
	dmx_serialise_array(char *, variableValues);
	dmx_serialise_array(char *, variableTypeNames);
	dmx_serialise_array(char *, variableTypeParameters);
	dmx_serialise_array(void, m_ChoiceGroups);
};

dmx_serialise struct CMapSelectionSet
{
	dmx_serialise_array(CMapEntity, children);
	char *selectionSetName;
	DmxElementId selectionSetData;
};

dmx_serialise struct CMapWorld
{
	dmx_serialise_array(CMapEntity, children);
	dmx_serialise_array(char *, variableTargetKeys);
	dmx_serialise_array(char *, variableNames);
	dmx_serialise_array(DmePlugList, relayPlugData);
	dmx_serialise_array(void, connectionsData);
	EditGameClassProps entity_properties; // TODO: how to do this?
	s32 nextDecalID;
	bool fixupEntityNames;
	char *mapUsageType;
	v3 origin;
	QAngle angles;
	v3 scales;
	bool transformLocked;
	bool force_hidden;
	bool editorOnly;
};

dmx_serialise struct CMapRootElement
{
	bool isprefab;
	s32 editorbuild;
	s32 editorversion;
	char *itemFile;
	CStoredCamera defaultcamera;
	CStoredCameras dmx_name_override(3dcameras) cameras;
	CMapWorld world;
	CVisibilityMgr visbility;
	CMapVariableSet mapVariables;
	CMapSelectionSet rootSelectionSet;
	dmx_serialise_array(void, m_ReferencedMeshSnapshots);
	bool m_bIsCordoning;
	bool m_bCordonsVisible;
	dmx_serialise_array(void, nodeInstanceData);
};

#endif //VMAP_TYPES_H
