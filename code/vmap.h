/* date = December 13th 2023 0:45 pm */

#ifndef VMAP_TYPES_H
#define VMAP_TYPES_H

typedef v3 QAngle;

// TODO: entity properties seem to all be encoded as strings
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

typedef struct
{
	const char name[64];
	const char standardAttributeName[64];
	const char semanticName[64];
	i32 semanticIndex;
	i32 vertexBufferLocation;
	i32 dataStateFlags; // TODO: what are the flags?
	//i32 subdivisionBinding; // type is "element"?
	DmxAttrValue data; // data type can be many things :(
} CDmePolygonMeshDataStream;

typedef struct
{
	i32 size; // equals streams[i].dataCount?
	i32 streamCount;
	CDmePolygonMeshDataStream *streams;
} CDmePolygonMeshDataArray;

// will never need subdivision for goldsrc > source 2 conversion
typedef struct
{
	i32 subdivisonLevelCount;
	i32 *subdivisionLevels;
	void *streams; // TODO: "element_array" type, need to get the actual data structure
} CDmePolygonMeshSubdivisionData;

typedef struct
{
	i32 vertexCount;
	i32 *vertexEdgeIndices;
	i32 *vertexDataIndices;
	
	i32 edgeCount;
	i32 *edgeVertexIndices;
	i32 *edgeOppositeIndices;
	i32 *edgeNextIndices;
	i32 *edgeFaceIndices;
	i32 *edgeDataIndices;
	i32 *edgeVertexDataIndices;
	
	i32 faceCount;
	i32 *faceEdgeIndices;
	i32 *faceDataIndices;
	
	i32 materialCount;
	const char **materials;
	
	CDmePolygonMeshDataArray vertexData;
	CDmePolygonMeshDataArray faceVertexData;
	CDmePolygonMeshDataArray edgeData;
	CDmePolygonMeshDataArray faceData;
	CDmePolygonMeshSubdivisionData subdivisionData;
} CDmePolygonMesh;

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
	i32 nodeID;
	u64 referenceID; // TODO: what's this
	void *children; // element_array
	void *variableTargetKeys; // element_array
	void *variableNames; // element_array
	const char *cubeMapName;
	const char *lightGroup;
	bool visexclude;
	bool disablemerging;
	bool renderwithdynamic;
	bool disableHeightDisplacement;
	f32 fademindist;
	f32 fademaxdist;
	bool bakelighting;
	bool precomputelightprobes;
	bool renderToCubemaps;
	bool emissiveLightingEnabled;
	float emissiveLightingBoost;
	int disableShadows;
	bool lightingDummy;
	f32 smoothingAngle;
	u32 tintColor; // TODO: rgba8888 struct?!
	i32 renderAmt;
	const char *physicsType;
	const char *physicsGroup;
	const char *physicsInteractsAs;
	const char *physicsInteractsWith;
	const char *physicsInteractsExclude;
	CDmePolygonMesh meshData;
	bool physicsSimplificationOverride;
	f32 physicsSimplificationError;
	
	v3 origin;
	v3 angles;
	v3 scales;
	bool transformLocked;
	bool force_hidden;
	bool editorOnly;
} CMapMesh;

typedef struct
{
	// TODO: these need counts
	void *children; // CMapMesh and CMapEntity, maybe more?
	char *variableTargetKeys; // element_array
	char *variableNames; // element_array
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

typedef struct
{
	i32 unused;
} Vmap;

#endif //VMAP_TYPES_H
