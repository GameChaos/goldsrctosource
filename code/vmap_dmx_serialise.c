
// AUTO GENERATED, DO NOT EDIT!

DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxSerialiseArrayDmePlugList, DmePlugList, DMX_ATTR_ELEMENT_ARRAY);

internal void DmxSerialiseDmePlugList(Dmx *dmx, DmxElement *parent, str name, DmePlugList value, Arena *arena)
{
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("names"), value.names);
	}
	{
		DmxElement *elem = DmxAddAttributeInt(dmx, parent, STR("dataTypes"), value.dataTypes);
	}
	{
		DmxElement *elem = DmxAddAttributeInt(dmx, parent, STR("plugTypes"), value.plugTypes);
	}
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("descriptions"), value.descriptions);
	}
}

DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxSerialiseArrayCStoredCamera, CStoredCamera, DMX_ATTR_ELEMENT_ARRAY);

internal void DmxSerialiseCStoredCamera(Dmx *dmx, DmxElement *parent, str name, CStoredCamera value, Arena *arena)
{
	{
		DmxElement *elem = DmxAddAttributeV3(dmx, parent, STR("position"), value.position);
	}
	{
		DmxElement *elem = DmxAddAttributeV3(dmx, parent, STR("lookat"), value.lookat);
	}
}

DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxSerialiseArrayCStoredCameras, CStoredCameras, DMX_ATTR_ELEMENT_ARRAY);

internal void DmxSerialiseCStoredCameras(Dmx *dmx, DmxElement *parent, str name, CStoredCameras value, Arena *arena)
{
	{
		DmxElement *elem = DmxAddAttributeInt(dmx, parent, STR("activecamera"), value.activecamera);
	}
}

DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxSerialiseArrayCMapEntity, CMapEntity, DMX_ATTR_ELEMENT_ARRAY);

internal void DmxSerialiseCMapEntity(Dmx *dmx, DmxElement *parent, str name, CMapEntity value, Arena *arena)
{
	{
		DmxElement *elem = DmxAddAttributeV3(dmx, parent, STR("hitNormal"), value.hitNormal);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("isProceduralEntity"), value.isProceduralEntity);
	}
	{
		DmxElement *elem = DmxAddAttributeV3(dmx, parent, STR("origin"), value.origin);
	}
	{
		DmxElement *elem = DmxAddAttributeQAngle(dmx, parent, STR("angles"), value.angles);
	}
	{
		DmxElement *elem = DmxAddAttributeV3(dmx, parent, STR("scales"), value.scales);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("transformLocked"), value.transformLocked);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("force_hidden"), value.force_hidden);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("editorOnly"), value.editorOnly);
	}
}

DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxSerialiseArrayCMapGroup, CMapGroup, DMX_ATTR_ELEMENT_ARRAY);

internal void DmxSerialiseCMapGroup(Dmx *dmx, DmxElement *parent, str name, CMapGroup value, Arena *arena)
{
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("variableTargetKeys"), value.variableTargetKeys);
	}
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("variableNames"), value.variableNames);
	}
	{
		DmxElement *elem = DmxAddAttributeV3(dmx, parent, STR("origin"), value.origin);
	}
	{
		DmxElement *elem = DmxAddAttributeQAngle(dmx, parent, STR("angles"), value.angles);
	}
	{
		DmxElement *elem = DmxAddAttributeV3(dmx, parent, STR("scales"), value.scales);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("transformLocked"), value.transformLocked);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("force_hidden"), value.force_hidden);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("editorOnly"), value.editorOnly);
	}
}

DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxSerialiseArrayCVisibilityMgr, CVisibilityMgr, DMX_ATTR_ELEMENT_ARRAY);

internal void DmxSerialiseCVisibilityMgr(Dmx *dmx, DmxElement *parent, str name, CVisibilityMgr value, Arena *arena)
{
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("variableTargetKeys"), value.variableTargetKeys);
	}
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("variableNames"), value.variableNames);
	}
	{
		DmxElement *elem = DmxAddAttributeV3(dmx, parent, STR("origin"), value.origin);
	}
	{
		DmxElement *elem = DmxAddAttributeQAngle(dmx, parent, STR("angles"), value.angles);
	}
	{
		DmxElement *elem = DmxAddAttributeV3(dmx, parent, STR("scales"), value.scales);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("transformLocked"), value.transformLocked);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("force_hidden"), value.force_hidden);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("editorOnly"), value.editorOnly);
	}
}

DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxSerialiseArrayCMapVariableSet, CMapVariableSet, DMX_ATTR_ELEMENT_ARRAY);

internal void DmxSerialiseCMapVariableSet(Dmx *dmx, DmxElement *parent, str name, CMapVariableSet value, Arena *arena)
{
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("variableNames"), value.variableNames);
	}
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("variableValues"), value.variableValues);
	}
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("variableTypeNames"), value.variableTypeNames);
	}
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("variableTypeParameters"), value.variableTypeParameters);
	}
}

DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxSerialiseArrayCMapSelectionSet, CMapSelectionSet, DMX_ATTR_ELEMENT_ARRAY);

internal void DmxSerialiseCMapSelectionSet(Dmx *dmx, DmxElement *parent, str name, CMapSelectionSet value, Arena *arena)
{
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("selectionSetName"), value.selectionSetName);
	}
	{
		DmxElement *elem = DmxAddAttributeElementId(dmx, parent, STR("selectionSetData"), value.selectionSetData);
	}
}

DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxSerialiseArrayCMapWorld, CMapWorld, DMX_ATTR_ELEMENT_ARRAY);

internal void DmxSerialiseCMapWorld(Dmx *dmx, DmxElement *parent, str name, CMapWorld value, Arena *arena)
{
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("variableTargetKeys"), value.variableTargetKeys);
	}
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("variableNames"), value.variableNames);
	}
	{
		DmxElement *elem = DmxAddAttributeInt(dmx, parent, STR("nextDecalID"), value.nextDecalID);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("fixupEntityNames"), value.fixupEntityNames);
	}
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("mapUsageType"), value.mapUsageType);
	}
	{
		DmxElement *elem = DmxAddAttributeV3(dmx, parent, STR("origin"), value.origin);
	}
	{
		DmxElement *elem = DmxAddAttributeQAngle(dmx, parent, STR("angles"), value.angles);
	}
	{
		DmxElement *elem = DmxAddAttributeV3(dmx, parent, STR("scales"), value.scales);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("transformLocked"), value.transformLocked);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("force_hidden"), value.force_hidden);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("editorOnly"), value.editorOnly);
	}
}

DEFINE_DMXADDATTRIBUTEARRAY_FUNC(DmxSerialiseArrayCMapRootElement, CMapRootElement, DMX_ATTR_ELEMENT_ARRAY);

internal void DmxSerialiseCMapRootElement(Dmx *dmx, DmxElement *parent, str name, CMapRootElement value, Arena *arena)
{
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("isprefab"), value.isprefab);
	}
	{
		DmxElement *elem = DmxAddAttributeInt(dmx, parent, STR("editorbuild"), value.editorbuild);
	}
	{
		DmxElement *elem = DmxAddAttributeInt(dmx, parent, STR("editorversion"), value.editorversion);
	}
	{
		DmxElement *elem = DmxAddAttributeString(dmx, parent, STR("itemFile"), value.itemFile);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("m_bIsCordoning"), value.m_bIsCordoning);
	}
	{
		DmxElement *elem = DmxAddAttributeBool(dmx, parent, STR("m_bCordonsVisible"), value.m_bCordonsVisible);
	}
}

