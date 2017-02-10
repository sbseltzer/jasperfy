#include <orx.h>
#include <string>

struct MapParser {

public:
  orxVECTOR gridPosition;
  orxVECTOR worldPosition;
  orxSTRING tileSection;

private:
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
    for (orxU32 keyIndex = 0; keyIndex < numKeys; keyIndex++) {
      const orxSTRING key = orxConfig_GetKey(keyIndex);
      orxLOG("attempting to load tileID %s", key);
      // Make sure the key value is a valid section
      const orxSTRING value = orxConfig_GetString(key);
      if (orxConfig_HasSection(value)) {
        orxHashTable_Add(tileTable, (orxU64) orxString_GetID(key), (void *) (orxU64) value);
        orxLOG("%s -> %s", key, value);
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
      orxLOG("Skipping whitespace: mapString[%u] = '%c'", index, ch);
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
      orxLOG("Reading tileID: mapString[%u] = '%c'", index, ch);
      ch = mapString[++index];
    }
    std::string tileID = mapString.substr(tileStartIndex, index - tileStartIndex);
    tileSection = (orxSTRING)orxHashTable_Get(tileTable, (orxU64)orxString_GetID(tileID.c_str()));
    orxLOG("tileID is substring from %u to %u: %s %s", tileStartIndex, index, tileID.c_str(), tileSection);
    return ( index < length );
  }

public:
  MapParser() :
    tileTable((orxHASHTABLE*)orxNULL),
    gridPosition(orxVECTOR_0),
    worldPosition(orxVECTOR_0),
    tileSection((orxSTRING)orxNULL),
    index(0), length(0), gridSize(0), mapString("") {}

  orxBOOL Setup(const orxSTRING mapSection) {
    index = 0;
    orxVector_Set(&gridPosition, 0.0, 0.0, 0.0);
    orxVector_Set(&worldPosition, 0.0, 0.0, 0.0);
    tileSection = (orxSTRING) orxNULL;

    readConfig(mapSection);
    length = orxString_GetLength(mapString.c_str());

    orxLOG("mapString: %u\n\"%s\"", length, mapString.c_str());
  }

  orxBOOL nextTile() {
    orxLOG("Parsing next tile");
    // Skip whitespace, keeping a count of linebreaks along the way.
    orxU32 lineBreaks = 0;
    orxBOOL continueParsing = skipWhiteSpace(lineBreaks);
    orxLOG("lineBreaks: %u", lineBreaks);
    gridPosition.fY += lineBreaks;
    // If a linebreak was hit, reset tilespace column to 0.
    if (lineBreaks > 0) { gridPosition.fX = 0; }
    else { gridPosition.fX++; }
    // If we hit the end of the string, return as such.
    if (!continueParsing) { return orxFALSE; }
    // Update the world position
    worldPosition.fX = gridPosition.fX * gridSize;
    worldPosition.fY = gridPosition.fY * gridSize;
    // Update current tile ID and return false if it detects end of string.
    return updateTileID();
  }
};

// Implementation of map generation
void readConfig(const orxSTRING mapName) {
  if (mapName != orxNULL) {
    MapParser parser;
    parser.Setup(mapName);
    while (parser.nextTile()) {
      if (parser.tileSection == orxNULL) continue;
      orxOBJECT *object = orxObject_CreateFromConfig(parser.tileSection);
      orxObject_SetPosition(object, &parser.worldPosition);
      orxLOG("Pos<%f,%f,%f> %s", parser.gridPosition.fX, parser.gridPosition.fY, parser.gridPosition.fZ, parser.tileSection);
    }
  }
}
