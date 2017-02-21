#include <orx.h>
#include <string>

/*! Absolute minimum grid size */
static const orxU32 MAP_MIN_GRIDSIZE = 1;
/*! Minimum grid size for processing physics */
static const orxU32 MAP_MIN_GRIDSIZE_PHYSICS = 8;

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
  return (MapObjectParser) {_zMapName, 0};
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
      orxLOG("On key %d (%s)", _pstParser->u32KeyIndex, zKeyName);
      *_zKeyOut = zKeyName;
      status = orxSTATUS_SUCCESS;
      break;
    }
  }
  orxConfig_PopSection();
  return status;
}

// Implementation of map generation
void loadMapData(const orxSTRING mapName) {
  if (mapName != orxNULL) {
    MapParser parser(mapName);
    const orxOBJECT *map = orxObject_CreateFromConfig(mapName);
    orxBODY *body = orxOBJECT_GET_STRUCTURE(map, BODY);
    while (parser.nextTile()) {
      if (parser.tileSection == orxNULL) continue;
      map_AddPhysicsForTile(parser.tileSection, body, parser.getGridSize(), &parser.tileTopLeft);
      orxOBJECT *object = orxObject_CreateFromConfig(parser.tileSection);
      orxObject_SetPosition(object, &parser.tileTopLeft);
      // orxLOG("Pos<%f,%f,%f> %s", parserData.gridPosition.fX, parserData.gridPosition.fY, parserData.gridPosition.fZ, parserData._zTileName);
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
          orxLOG("Creating object[%d] for postition %s = %s", i, objectListKey, objectListValue);
          orxOBJECT *object = orxObject_CreateFromConfig(objectListValue);
          orxObject_SetPosition(object, &objectPos);
        }
      }
      orxConfig_PopSection();
    }
  }
}
