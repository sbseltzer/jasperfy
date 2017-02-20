#include <orx.h>
#include <string>

struct MapParser {

public:
  orxVECTOR gridPosition;
  orxVECTOR tileTopLeft;
  orxVECTOR tileCenter;
  orxVECTOR tileBottomRight;
  orxSTRING tileSection;

private:
  const orxSTRING mapName;
  orxU32 index;
  orxU32 length;
  orxU32 gridSize;
  std::string mapString;
  orxHASHTABLE *tileTable;

  void loadTilesIDs() {
    const orxU32 numKeys = orxConfig_GetKeyCounter();
    if (tileTable == orxNULL) {
      tileTable = orxHashTable_Create(numKeys, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    } else {
      orxHashTable_Clear(tileTable);
    }
    // Populate table
    for (orxU32 tileKeyIndex = 0; tileKeyIndex < numKeys; tileKeyIndex++) {
      const orxSTRING tileKey = orxConfig_GetKey(tileKeyIndex);
      // orxLOG("attempting to load tileID %s", tileKey);
      // Make sure the key value is a valid section
      const orxSTRING tileValue = orxConfig_GetString(tileKey);
      if (orxConfig_HasSection(tileValue)) {
        orxHashTable_Add(tileTable, (orxU64) orxString_GetID(tileKey), (void *) (orxU64) tileValue);
        orxConfig_PushSection(tileValue);
        if (orxConfig_HasValue("Body")) {
          // orxHashTable_Add(tileBodyTable, (orxU64) orxString_GetID(tileKey), (void *) (orxU64) tileValue);
        }
        orxConfig_PopSection();
        // orxLOG("%s -> %s", tileKey, tileValue);
      }
    }
  }

  void readConfig(const orxSTRING sectionName) {
    const orxSTRING tileTableSection;

    // Read map info from config
    orxConfig_PushSection(sectionName);
    gridSize = orxConfig_GetU32("GridSize");
    mapString = orxConfig_GetString("Map");
    tileTableSection = orxConfig_GetString("Tiles");
    orxConfig_PopSection();

    // Populate the tile table using the section name specified by the map
    orxConfig_PushSection(tileTableSection);
    loadTilesIDs();
    orxConfig_PopSection();
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
    orxVector_Set(&tileCenter, 0.0, 0.0, 0.0);
    orxVector_Set(&tileBottomRight, 0.0, 0.0, 0.0);
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
    tileCenter.fX = tileTopLeft.fX + gridSize / 2;
    tileCenter.fY = tileTopLeft.fY + gridSize / 2;
    tileBottomRight.fX = tileTopLeft.fX + gridSize;
    tileBottomRight.fY = tileTopLeft.fY + gridSize;

    // Update current tile ID and return false if it detects end of string.
    continueParsing = updateTileID();
    if (!continueParsing) {
      orxConfig_PopSection();
    }
    return continueParsing;
  }
};

// Implementation of map generation
void loadMapData(const orxSTRING mapName) {
  if (mapName != orxNULL) {
    MapParser parser(mapName);
    const orxOBJECT *map = orxObject_CreateFromConfig(mapName);
    orxBODY *body = orxOBJECT_GET_STRUCTURE(map, BODY);

    while (parser.nextTile()) {
      if (parser.tileSection == orxNULL) continue;
      orxConfig_PushSection(parser.tileSection);
      if (orxString_Compare(orxConfig_GetString("Type"), "box") == 0) {
        orxConfig_SetVector("TopLeft", &parser.tileTopLeft);
        orxConfig_SetVector("BottomRight", &parser.tileBottomRight);
      }
      orxConfig_PopSection();
      orxBody_AddPartFromConfig(body, parser.tileSection);
      orxOBJECT *object = orxObject_CreateFromConfig(parser.tileSection);
      orxObject_SetPosition(object, &parser.tileTopLeft);
      // orxLOG("Pos<%f,%f,%f> %s", parser.gridPosition.fX, parser.gridPosition.fY, parser.gridPosition.fZ, parser.tileSection);
    }
  }
}
