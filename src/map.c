#include <map.h>

/*! Absolute minimum grid size */
static const orxU32 MAP_MIN_GRIDSIZE = 1;
/*! Minimum grid size for processing physics */
static const orxU32 MAP_MIN_GRIDSIZE_PHYSICS = 8;
/*! Max length of tile aliases */
static const orxU32 MAP_MAX_TILE_ALIAS_LENGTH = 64;

/*! Map storage */
static orxBANK *spstMapBank;
static orxLINKLIST sstMapList;

MapLayoutParser map_CreateLayoutParser(const MapData* pstMap) {
  MapLayoutParser parser = {pstMap, orxString_GetLength(pstMap->zMapLayout), 0};
  orxVector_Set(&parser.vCurrentGridPos, 0, 0, 0);
  orxVector_Set(&parser.vCurrentWorldPos, 0, 0, 0);
  parser.zCurrentTileName = (orxSTRING) orxNULL;
  return parser;
}

static orxBOOL map_ParseLayoutWhiteSpace(MapLayoutParser *pstParser, orxU32 *pu32LineBreaks) {
  if (pstParser->u32Index >= pstParser->u32Length) { return orxFALSE; }
  orxCHAR ch = pstParser->pstMap->zMapLayout[pstParser->u32Index];
  while ( ( ch == ' ' || ch == '\r' || ch == '\n' ) && pstParser->u32Index < pstParser->u32Length ) {
    // orxLOG("Skipping whitespace: pstParser->pstMap->zMapLayout[%u] = '%c'", pstParser->u32Index, ch);
    if (ch == '\n') { (*pu32LineBreaks)++; }
    ch = pstParser->pstMap->zMapLayout[++pstParser->u32Index];
  }
  return ( pstParser->u32Index < pstParser->u32Length );
}

static orxBOOL map_ParseLayoutTile(MapLayoutParser *pstParser) {
  if (pstParser->u32Index >= pstParser->u32Length) { return orxFALSE; }
  orxCHAR ch = pstParser->pstMap->zMapLayout[pstParser->u32Index];
  orxU32 u32StartIndex = pstParser->u32Index;
  while (ch != ' ' && ch != '\r' && ch != '\n' && pstParser->u32Index < pstParser->u32Length) {
    // orxLOG("Reading tileID: mapString[%u] = '%c'", pstParser->u32Index, ch);
    ch = pstParser->pstMap->zMapLayout[++pstParser->u32Index];
  }
  orxU32 u32SubLength = pstParser->u32Index - u32StartIndex;
  orxASSERT(u32SubLength < MAP_MAX_TILE_ALIAS_LENGTH);
  orxCHAR zTileAlias[MAP_MAX_TILE_ALIAS_LENGTH];
  strncpy(zTileAlias, pstParser->pstMap->zMapLayout + u32StartIndex, u32SubLength);
  zTileAlias[u32SubLength] = '\0';
  pstParser->zCurrentTileName = (orxSTRING)orxHashTable_Get(pstParser->pstMap->pstTileAliasTable, (orxU64)orxString_GetID(zTileAlias));
  // orxLOG("tileID is substring from %u to %u: %s %s", u32StartIndex, pstParser->u32Index, zTileAlias, pstParser->zCurrentTileName);
  return ( pstParser->u32Index < pstParser->u32Length );
}

orxBOOL map_ParseLayout(MapLayoutParser *pstParser) {
  // orxLOG("Parsing next tile");
  // Skip whitespace, keeping a count of linebreaks along the way.
  orxU32 u32LineBreaks = 0;
  orxBOOL bContinue = map_ParseLayoutWhiteSpace(pstParser, &u32LineBreaks);

  // orxLOG("u32LineBreaks: %u", u32LineBreaks);
  pstParser->vCurrentGridPos.fY += u32LineBreaks;

  // If a linebreak was hit, reset tilespace column to 0.
  if (u32LineBreaks > 0) { pstParser->vCurrentGridPos.fX = 0; }
  else { pstParser->vCurrentGridPos.fX++; }

  // Update the world positions
  pstParser->vCurrentWorldPos.fX = pstParser->vCurrentGridPos.fX * pstParser->pstMap->vGridUnitSize.fX;
  pstParser->vCurrentWorldPos.fY = pstParser->vCurrentGridPos.fY * pstParser->pstMap->vGridUnitSize.fY;

  // orxLOG("Tile Position: <%f %f %f> <%f %f %f>",
  //        pstParser->vCurrentGridPos.fX,
  //        pstParser->vCurrentGridPos.fY,
  //        pstParser->vCurrentGridPos.fZ,
  //        pstParser->vCurrentWorldPos.fX,
  //        pstParser->vCurrentWorldPos.fY,
  //        pstParser->vCurrentWorldPos.fZ);
  // If we hit the end of the string, return as such.
  if (!bContinue) { return orxFALSE; }

  // Update current tile ID and return false if it detects end of string.
  bContinue = map_ParseLayoutTile(pstParser);
  if (!bContinue) {
    orxConfig_PopSection();
  }
  return bContinue;
}

void map_AddPhysicsForTile(const orxSTRING _zTileName, orxBODY *_pstWorldBody, orxVECTOR *vGridUnitSize, orxVECTOR *_vTopLeft) {
  const orxSTRING zPartType;
  if (vGridUnitSize->fX < MAP_MIN_GRIDSIZE_PHYSICS || vGridUnitSize->fY < MAP_MIN_GRIDSIZE_PHYSICS) {
    orxLOG("WARNING: Attempted to add physics for tile with vGridUnitSize of %f,%f (<= %u)! Doing so is unsafe. Skipping...", vGridUnitSize->fX, vGridUnitSize->fY, MAP_MIN_GRIDSIZE_PHYSICS);
    return;
  }
  orxASSERT(_zTileName);
  orxASSERT(_pstWorldBody);
  orxConfig_PushSection(_zTileName);
  zPartType = orxConfig_GetString("Type");
  if (orxString_Compare(zPartType, "") != 0) {
    if (orxString_Compare(zPartType, "box") == 0) {
      orxConfig_SetVector("TopLeft", _vTopLeft);
      orxVECTOR bottomRight = {0};
      orxVector_Set(&bottomRight, _vTopLeft->fX + vGridUnitSize->fX, _vTopLeft->fY + vGridUnitSize->fY, 0);
      orxConfig_SetVector("BottomRight", &bottomRight);
    } else if (orxString_Compare(zPartType, "sphere") == 0) {
      orxVECTOR center = {0};
      orxVector_Set(&center, _vTopLeft->fX + vGridUnitSize->fX / 2.0, _vTopLeft->fY + vGridUnitSize->fY / 2.0, 0);
      orxConfig_SetVector("BottomRight", &center);
    } else if (orxString_Compare(zPartType, "mesh") == 0) {
      orxLOG("Mesh type unsupported: %s", _zTileName);
    }
    orxBody_AddPartFromConfig(_pstWorldBody, _zTileName);
  }
  orxConfig_PopSection();
}

MapObjectParser map_CreateObjectParser(const orxSTRING _zMapName) {
  MapObjectParser parser = {_zMapName, 0};
  return parser;
}

orxSTATUS map_NextObjectListPosition(MapObjectParser *_pstParser, const orxSTRING *_zKeyOut, orxVECTOR *vPositionOut) {
  orxSTATUS status = orxSTATUS_FAILURE;
  orxConfig_PushSection(_pstParser->zMapName);
  orxVector_Set(vPositionOut, 0, 0, 0);
  const orxU32 u32KeyCount = orxConfig_GetKeyCounter();
  while ((_pstParser->u32KeyIndex < u32KeyCount)) {
    const orxSTRING zKeyName = orxConfig_GetKey(_pstParser->u32KeyIndex);
    _pstParser->u32KeyIndex++;
    if (orxString_ToVector(zKeyName, vPositionOut, orxNULL) == orxSTATUS_SUCCESS) {
      *_zKeyOut = zKeyName;
      status = orxSTATUS_SUCCESS;
      break;
    }
  }
  orxConfig_PopSection();
  return status;
}

orxHASHTABLE* map_LoadTileTable(const orxSTRING _zName) {
  orxU32 numKeys;
  orxHASHTABLE *pstTileTable;
  orxConfig_PushSection(_zName);
  numKeys = orxConfig_GetKeyCounter();
  orxASSERT(numKeys > 0);
  pstTileTable = orxHashTable_Create(numKeys, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
  orxASSERT(pstTileTable);
  orxLOG("created hashtable with %u keys", numKeys);
  // Populate table
  for (orxU32 tileKeyIndex = 0; tileKeyIndex < numKeys; tileKeyIndex++) {
    const orxSTRING tileKey = orxConfig_GetKey(tileKeyIndex);
    orxLOG("attempting to load tileID %s", tileKey);
    // Make sure the key value is a valid section
    const orxSTRING tileValue = orxConfig_GetString(tileKey);
    if (orxConfig_HasSection(tileValue)) {
      orxLOG("%s -> %s", tileKey, tileValue);
      orxHashTable_Add(pstTileTable, (orxU64) orxString_GetID(tileKey), (void *) (orxU64) tileValue);
    }
  }
  orxConfig_PopSection();
  return pstTileTable;
}

MapData *map_CreateMapData(const orxSTRING _zMapName) {
  MapData *pstMap = orxNULL;

  orxASSERT(_zMapName);

  pstMap = (MapData *)orxBank_Allocate(spstMapBank);
  orxASSERT(pstMap);

  // known values
  pstMap->zMapName = _zMapName;
  orxConfig_PushSection(pstMap->zMapName);
  pstMap->zTileAliasTableName = orxConfig_GetString("Tiles");
  pstMap->zBodyName = orxConfig_GetString("Body");
  pstMap->zMapLayout = orxConfig_GetString("Map");
  orxConfig_GetVector("GridSize", &pstMap->vGridUnitSize);
  orxConfig_PopSection();

  // unknowns
  orxVector_Set(&pstMap->vGridDimensions, 0, 0, 0);
  pstMap->pstBody = orxNULL;
  pstMap->pstTileAliasTable = orxNULL;
  pstMap->pstTileIndexTable = orxNULL;
  pstMap->pstObjectPosTable = orxNULL;

  orxLinkList_AddEnd(&sstMapList, (orxLINKLIST_NODE *)pstMap);

  pstMap->pstTileAliasTable = map_LoadTileTable(pstMap->zTileAliasTableName);

  return pstMap;
}

void map_DeleteMapData(MapData *_pstMap) {
  orxASSERT(_pstMap);
  orxObject_SetLifeTime(_pstMap->pstObject, orxFLOAT_0);
  orxLinkList_Remove((orxLINKLIST_NODE *)_pstMap);
  orxBank_Free(spstMapBank, _pstMap);
}

void map_Generate(MapData *pstMapData) {
  MapLayoutParser stParser = map_CreateLayoutParser(pstMapData);
  pstMapData->pstObject = orxObject_CreateFromConfig(pstMapData->zMapName);
  pstMapData->pstBody = orxOBJECT_GET_STRUCTURE(pstMapData->pstObject, BODY);
  while (map_ParseLayout(&stParser)) {
    if (stParser.zCurrentTileName == orxNULL) continue;
    map_AddPhysicsForTile(stParser.zCurrentTileName, pstMapData->pstBody, &pstMapData->vGridUnitSize, &stParser.vCurrentWorldPos);
    orxOBJECT *object = orxObject_CreateFromConfig(stParser.zCurrentTileName);
    orxObject_SetPosition(object, &stParser.vCurrentWorldPos);
    orxObject_SetOwner(object, pstMapData->pstObject);
  }
  MapObjectParser objectParser = map_CreateObjectParser(pstMapData->zMapName);
  orxVECTOR objectPos = {0};
  const orxSTRING objectListKey;
  while (map_NextObjectListPosition(&objectParser, &objectListKey, &objectPos) != orxSTATUS_FAILURE) {
    objectPos.fX *= pstMapData->vGridUnitSize.fX;
    objectPos.fY *= pstMapData->vGridUnitSize.fY;
    objectPos.fZ *= pstMapData->vGridUnitSize.fZ;
    orxConfig_PushSection(objectParser.zMapName);
    orxS32 numObjects = orxConfig_GetListCounter(objectListKey);
    for (orxS32 i = 0; i < numObjects; i++) {
      const orxSTRING objectListValue = orxConfig_GetListString(objectListKey, i);
      if (orxConfig_HasSection(objectListValue)) {
        orxOBJECT *object = orxObject_CreateFromConfig(objectListValue);
        orxObject_SetPosition(object, &objectPos);
        orxObject_SetOwner(object, pstMapData->pstObject);
      }
    }
    orxConfig_PopSection();
  }
}

void map_Reload(MapData *_pstMap) {
  const orxSTRING zMapName = _pstMap->zMapName;
  map_DeleteMapData(_pstMap);
  *_pstMap = *(map_CreateMapData(zMapName));
  map_Generate(_pstMap);
  // return _pstMap;
}

orxSTATUS orxFASTCALL map_ConfigEventHandler(const orxEVENT *_pstEvent) {
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  if (_pstEvent->eID == orxRESOURCE_EVENT_UPDATE) {
    orxRESOURCE_EVENT_PAYLOAD *pstPayload = (orxRESOURCE_EVENT_PAYLOAD *)_pstEvent->pstPayload;
    orxLOG("Update: %s", orxString_GetFromID(pstPayload->u32NameID));
    orxLINKLIST_NODE *pstNode = (orxLINKLIST_NODE *)sstMapList.pstFirst;
    while (pstNode != orxNULL) {
      MapData *pstMap = (MapData *)pstNode;
      const orxU32 zOriginID = orxConfig_GetOriginID(pstMap->zMapName);
      if (zOriginID == pstPayload->u32NameID) {
        map_Reload(pstMap);
      }
      pstNode = orxLinkList_GetNext(pstNode);
    }
  }
  return eResult;
}

orxSTATUS map_Init() {
  orxSTATUS result = orxSTATUS_SUCCESS;
  spstMapBank = orxBank_Create(32, sizeof(MapData), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
  orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, map_ConfigEventHandler);
  return result;
}

orxSTATUS map_Exit() {
  orxSTATUS result = orxSTATUS_SUCCESS;
  orxBank_Delete(spstMapBank);
  orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, map_ConfigEventHandler);
  return result;
}

// Implementation of map generation
void map_LoadMapData(const orxSTRING mapName) {
  if (mapName != orxNULL) {
    MapData* pstMapData = map_CreateMapData(mapName);
    map_Generate(pstMapData);
  }
}
