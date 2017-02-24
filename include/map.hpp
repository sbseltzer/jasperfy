#include <orx.h>
#include <string>

/*! Absolute minimum grid size */
static const orxU32 MAP_MIN_GRIDSIZE = 1;
/*! Minimum grid size for processing physics */
static const orxU32 MAP_MIN_GRIDSIZE_PHYSICS = 8;

/*! Map storage */
static orxBANK *spstMapBank;
static orxLINKLIST sstMapList;

/*!  */
typedef struct MapData {
  orxLINKLIST_NODE stNode;         /*! Mandatory for orxLINKLIST compatibility */
  const orxSTRING zMapName;        /*! Section name of map */
  const orxSTRING zTileAliasTableName;  /*! Section name to generate pstTileAliasTable */
  const orxSTRING zBodyName;       /*! Section name of the map's body definition */
  const orxSTRING zMapLayout;      /*! Map layout string to be parsed */
  orxVECTOR vGridUnitSize;         /*! The size of one grid unit */
  orxVECTOR vGridDimensions;       /*! The total size of the map in grid units */
  orxHASHTABLE *pstTileAliasTable;  /*! alias -> tile section name */
  orxHASHTABLE *pstTileIndexTable; /*! tile alias -> tile index */
  orxHASHTABLE *pstObjectPosTable; /*! position -> object list */
  orxBODY* pstBody;                /*! Body to add tile BodyParts to */
} MapData;

orxU64 map_HasChanged(MapData* _pstMap) {
  // My thought was to generate a hash of the map and all its dependencies, but I'm realizing that could explode in calculation time. It also would require re-checking all data related to all maps.
  // I'd need to check the tile table name, the body name, the map layout, the contents of the tile table, the contents of the body, the grid size, and the object table
}

/*! Load the specified section into a hashtable */
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
  orxLinkList_Remove((orxLINKLIST_NODE *)_pstMap);
  orxBank_Free(spstMapBank, _pstMap);
}

void map_Reload(MapData *_pstMap) {
  const orxSTRING zMapName = _pstMap->zMapName;
  map_DeleteMapData(_pstMap);
  *_pstMap = *(map_CreateMapData(zMapName));
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


struct MapParser {

public:
  orxVECTOR gridPosition;
  orxVECTOR tileTopLeft;
  orxSTRING tileSection;

private:
  const orxSTRING mapName;
  orxU32 index;
  orxU32 length;
  orxU32 gridSize;
  std::string mapString;
  orxHASHTABLE *tileTable;

  void readConfig(const orxSTRING sectionName) {
    const orxSTRING tileTableSection;

    // Read map info from config
    orxConfig_PushSection(sectionName);
    gridSize = orxConfig_GetU32("GridSize");
    mapString = orxConfig_GetString("Map");
    tileTableSection = orxConfig_GetString("Tiles");
    orxConfig_PopSection();

    // Populate the tile table using the section name specified by the map
    tileTable = map_LoadTileTable(tileTableSection);
    orxASSERT(tileTable);
  }

  orxBOOL skipWhiteSpace(orxU32 &lineBreaks) {
    if (index >= length) { return orxFALSE; }
    orxCHAR ch = mapString[index];
    while ( ( ch == ' ' || ch == '\r' || ch == '\n' ) && index < length ) {
      // orxLOG("Skipping whitespace: mapString[%u] = '%c'", index, ch);
      if (ch == '\n') { lineBreaks++; }
      ch = mapString[++index];
    }
    return ( index < length );
  }

  // Assumes skipWhiteSpace has been called before it.
  // If the index is on whitespace, it simply won't make any progress.
  orxBOOL updateTileID() {
    if (index >= length) { return orxFALSE; }
    orxCHAR ch = mapString[index];
    orxU32 tileStartIndex = index;
    while (ch != ' ' && ch != '\r' && ch != '\n' && index < length) {
      // orxLOG("Reading tileID: mapString[%u] = '%c'", index, ch);
      ch = mapString[++index];
    }
    std::string tileID = mapString.substr(tileStartIndex, index - tileStartIndex);
    tileSection = (orxSTRING)orxHashTable_Get(tileTable, (orxU64)orxString_GetID(tileID.c_str()));
    // orxLOG("tileID is substring from %u to %u: %s %s", tileStartIndex, index, tileID.c_str(), tileSection);
    return ( index < length );
  }

  orxBOOL Setup() {
    index = 0;
    orxVector_Set(&gridPosition, 0.0, 0.0, 0.0);
    orxVector_Set(&tileTopLeft, 0.0, 0.0, 0.0);
    tileSection = (orxSTRING) orxNULL;

    readConfig(mapName);
    length = orxString_GetLength(mapString.c_str());

    // orxLOG("mapString: %u\n\"%s\"", length, mapString.c_str());
    orxConfig_PushSection(mapName);
    return true;
  }

public:
  MapParser(const orxSTRING mapSection) :
    mapName(mapSection),
    tileTable((orxHASHTABLE*)orxNULL),
    gridPosition(orxVECTOR_0),
    tileTopLeft(orxVECTOR_0),
    tileSection((orxSTRING)orxNULL),
    index(0), length(0), gridSize(0), mapString("") {
    Setup();
  }

  orxU32 getGridSize() {
    return gridSize;
  }
  orxBOOL nextTile() {
    // orxLOG("Parsing next tile");
    // Skip whitespace, keeping a count of linebreaks along the way.
    orxU32 lineBreaks = 0;
    orxBOOL continueParsing = skipWhiteSpace(lineBreaks);
    // orxLOG("lineBreaks: %u", lineBreaks);
    gridPosition.fY += lineBreaks;
    // If a linebreak was hit, reset tilespace column to 0.
    if (lineBreaks > 0) { gridPosition.fX = 0; }
    else { gridPosition.fX++; }
    // If we hit the end of the string, return as such.
    if (!continueParsing) { return orxFALSE; }
    // Update the world positions
    tileTopLeft.fX = gridPosition.fX * gridSize;
    tileTopLeft.fY = gridPosition.fY * gridSize;

    // Update current tile ID and return false if it detects end of string.
    continueParsing = updateTileID();
    if (!continueParsing) {
      orxConfig_PopSection();
    }
    return continueParsing;
  }
};

void map_AddPhysicsForTile(const orxSTRING _zTileName, orxBODY *_pstWorldBody, orxU32 u32GridSize, orxVECTOR *_vTopLeft) {
  const orxSTRING zPartType;
  if (u32GridSize < MAP_MIN_GRIDSIZE_PHYSICS) {
    orxLOG("WARNING: Attempted to add physics for tile with u32GridSize of %u (<= %u)! Doing so is unsafe. Skipping...", u32GridSize, MAP_MIN_GRIDSIZE_PHYSICS);
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
      orxVector_Set(&bottomRight, _vTopLeft->fX + u32GridSize, _vTopLeft->fY + u32GridSize, 0);
      orxConfig_SetVector("BottomRight", &bottomRight);
    } else if (orxString_Compare(zPartType, "sphere") == 0) {
      orxVECTOR center = {0};
      orxVector_Set(&center, _vTopLeft->fX + u32GridSize / 2.0, _vTopLeft->fY + u32GridSize / 2.0, 0);
      orxConfig_SetVector("BottomRight", &center);
    } else if (orxString_Compare(zPartType, "mesh") == 0) {
      orxLOG("Mesh type unsupported: %s", _zTileName);
    }
    orxBody_AddPartFromConfig(_pstWorldBody, _zTileName);
  }
  orxConfig_PopSection();
}

typedef struct MapObjectParser {
  const orxSTRING zMapName;
  orxU32 u32KeyIndex;
} MapObjectParser;

MapObjectParser map_CreateObjectParser(const orxSTRING _zMapName) {
  return {_zMapName, 0};
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

// Implementation of map generation
void map_LoadMapData(const orxSTRING mapName) {
  if (mapName != orxNULL) {
    MapParser parser(mapName);
    const orxOBJECT *map = orxObject_CreateFromConfig(mapName);
    orxBODY *body = orxOBJECT_GET_STRUCTURE(map, BODY);
    while (parser.nextTile()) {
      if (parser.tileSection == orxNULL) continue;
      map_AddPhysicsForTile(parser.tileSection, body, parser.getGridSize(), &parser.tileTopLeft);
      orxOBJECT *object = orxObject_CreateFromConfig(parser.tileSection);
      orxObject_SetPosition(object, &parser.tileTopLeft);
    }
    MapObjectParser objectParser = map_CreateObjectParser(mapName);
    orxVECTOR objectPos = {0};
    const orxSTRING objectListKey;
    while (map_NextObjectListPosition(&objectParser, &objectListKey, &objectPos) != orxSTATUS_FAILURE) {
      orxVector_Mulf(&objectPos, &objectPos, parser.getGridSize());
      orxConfig_PushSection(objectParser.zMapName);
      orxS32 numObjects = orxConfig_GetListCounter(objectListKey);
      for (orxS32 i = 0; i < numObjects; i++) {
        const orxSTRING objectListValue = orxConfig_GetListString(objectListKey, i);
        if (orxConfig_HasSection(objectListValue)) {
          orxOBJECT *object = orxObject_CreateFromConfig(objectListValue);
          orxObject_SetPosition(object, &objectPos);
        }
      }
      orxConfig_PopSection();
    }
  }
}
