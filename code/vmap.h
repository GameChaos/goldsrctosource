/* date = December 13th 2023 0:45 pm */

#ifndef VMAP_TYPES_H
#define VMAP_TYPES_H

typedef v3 QAngle;

typedef struct
{
	i32 propertyCount;
	DmxAttribute *properties;
} EditGameClassProps;

typedef struct
{
	char *names;
	i32 dataTypes;
	i32 plugTypes;
	char *descriptions;
} DmePlugList;

typedef struct
{
	v3 position;
	v3 lookat;
} CStoredCamera;

typedef struct
{
	i32 activecamera;
	CStoredCamera cameras;
} CStoredCameras;

typedef struct CMapEntity_s CMapEntity;
struct CMapEntity_s
{
	// TODO: these need counts
	CMapEntity *children;
	void *variableTargetKeys;
	void *variableNames;
	void *connectionsData;
	EditGameClassProps entity_properties;
	v3 hitNormal;
	bool isProceduralEntity;
	v3 origin;
	QAngle angles;
	v3 scales;
	bool transformLocked;
	bool force_hidden;
	bool editorOnly;
};

typedef struct
{
	// TODO: these need counts
	CMapEntity *children;
	char *variableTargetKeys;
	char *variableNames;
	v3 origin;
	QAngle angles;
	v3 scales;
	bool transformLocked;
	bool force_hidden;
	bool editorOnly;
} CMapGroup;

typedef struct
{
	// TODO: these need counts
	CMapEntity *children;
	char *variableTargetKeys;
	char *variableNames;
	CMapGroup nodes;
	v3 origin;
	QAngle angles;
	v3 scales;
	bool transformLocked;
	bool force_hidden;
	bool editorOnly;
} CVisibilityMgr;

typedef struct
{
	// TODO: these need counts
	char *variableNames;
	char *variableValues;
	char *variableTypeNames;
	char *variableTypeParameters;
	void *m_ChoiceGroups;
} CMapVariableSet;

typedef struct
{
	// TODO: this needs a count
	CMapEntity *children;
	char *selectionSetName;
	DmxElementId selectionSetData;
} CMapSelectionSet;

typedef struct
{
	// TODO: these need counts
	CMapEntity *children;
	char *variableTargetKeys;
	char *variableNames;
	DmePlugList *relayPlugData;
	void *connectionsData;
	EditGameClassProps entity_properties;
	i32 nextDecalID;
	bool fixupEntityNames;
	char *mapUsageType;
	v3 origin;
	QAngle angles;
	v3 scales;
	bool transformLocked;
	bool force_hidden;
	bool editorOnly;
} CMapWorld;

typedef struct
{
	bool isprefab;
	i32 editorbuild;
	i32 editorversion;
	char *itemFile;
	CStoredCamera defaultcamera;
	// TODO: this needs a count
	CStoredCameras *cameras; // 3dcameras
	CMapWorld world;
	CVisibilityMgr visbility;
	CMapVariableSet mapVariables;
	CMapSelectionSet rootSelectionSet;
	// TODO: this needs a count
	void *m_ReferencedMeshSnapshots;
	bool m_bIsCordoning;
	bool m_bCordonsVisible;
	// TODO: this needs a count
	void *nodeInstanceData;
} CMapRootElement;

#endif //VMAP_TYPES_H
